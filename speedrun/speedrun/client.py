import requests 
from urllib.parse  import urljoin 
from .implant_pb2 import *

c2 = "http://127.0.0.1:5000"


task_list = "/task/list"
task_create = "task/create"
register = "/register"


def create_dummy_task(implant_id = 1, opcode = "exec" , args="whoami /priv" ):
    r = requests.post(
            urljoin(c2, task_create), 
            json = {
                "implant_id":implant_id,
                "opcode":opcode, 
                "args": args
                }
            )
    if r.status_code == 200:
        print(r.json())
    else:
        print("Sad!")

def create_dummy_implyn():
    r = Register()
    r.Password = "foobar"
    r.Username = "lol"
    r.Hostname = "Ghost"
    r.GUID = "xxxxxxxxxxxx"
    out = r.SerializeToString()
    r = reuqests.post(urljoin( c2, register), data = out)


#create_dummy_task()
