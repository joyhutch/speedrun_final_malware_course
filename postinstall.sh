sudo groupadd docker
sudo usermod -aG docker $USER 
newgrp docker
docker run hello-world

curl -sSL https://install.python-poetry.org | python3 -
export $PATH=$PATH:$HOME/.local/bin
sudo apt-get install -y postgresql-client
sudo apt install -y protobuf-compiler
wget https://jpa.kapsi.fi/nanopb/download/nanopb-0.4.8.tar.gz
tar xfv nanopb-0.4.8.tar.gz
