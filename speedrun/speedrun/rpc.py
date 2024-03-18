from flask import Blueprint, request , abort 
from speedrun.implant_pb2 import * 
from speedrun.db import db

rpc = Blueprint("rpc", __name__)


password = "foobar"

@rpc.route("/register", methods=["POST"])
def handle_register():
    register = Register()
    req_data = request.get_data()
    register.ParseFromString(req_data)
    if register.Password != password:
        abort(404)
    r = make_implant_from_reqquest(register)
    db.session.add(r)
    db.commit()
    print("New Implant connected!")
    return ""
    
    
@rpc.route("/testpb", methods=["POST"])
def handle_pbtest():
    register = Register()
    req_data = request.get_data()
    register.ParseFromString(req_data)
    print(register)
    return ""

