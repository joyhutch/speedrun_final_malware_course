#define PB_ENABLE_MALLOC 1
#include <debug.h>

#include <execute.h>
#include <httpclient.h>
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <stdio.h>

#include "implant.pb.h"

void test_pb() {
  Register registerMessage = Register_init_zero;
  registerMessage.Password = "myPassword";
  registerMessage.GUID = "1234-5678-ABCD";
  registerMessage.Username = "user1";
  registerMessage.Hostname = "host.local";

  //  size_t out = 0;
  uint8_t buffer[4096];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  bool status = pb_encode(&stream, Register_fields, &registerMessage);
  if (!status) {
    printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
  } // Adjust size as needed
  printf("wrote %llu\n", stream.bytes_written);
  // Decoding done, `registerMessage2` is now populated.

  size_t out_size = 0;
  LPBYTE result = HTTPRequest(L"POST", L"localhost", L"/testpb", 5000, L"test",
                              buffer, stream.bytes_written, &out_size, FALSE);
  printf("%s\n", result);
  if (out_size > 0) {
    free(result);
  }
  pb_istream_t istream = pb_istream_from_buffer(buffer, stream.bytes_written);
  Register registerMessage2 = Register_init_zero;
  status = pb_decode(&istream, Register_fields, &registerMessage2);
  if (!status) {
    printf("Decoding failed: %s\n", PB_GET_ERROR(&istream));
    return;
  }
  printf("Decoded Password: %s\n", (char *)registerMessage2.Password);
  printf("Decoded GUID: %s\n", (char *)registerMessage2.GUID);
  printf("Decoded Username: %s\n", (char *)registerMessage2.Username);
  printf("Decoded Hostname: %s\n", (char *)registerMessage2.Hostname);
  pb_release(Register_fields, &registerMessage2);
  printf("Done!\n");
}

int main() { test_pb(); }
