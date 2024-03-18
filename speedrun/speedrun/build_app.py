
from flask import Flask 
from speedrun.db import db 
from speedrun.admin import admin
from speedrun.rpc import rpc 
def build_app():
    app = Flask(__name__)
    app.config.from_mapping(
            SQLALCHEMY_DATABASE_URI = 'postgresql://speedrun:gofast@localhost:5432/src2'
#"sqlite:///c2.db"  
        
    )
    app.register_blueprint(admin)
    app.register_blueprint(rpc)
    db.init_app(app)
    return app 


def init_db():
    db.drop_all()
    db.create_all()
        


