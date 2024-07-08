# Running

```bash
docker compose build
docker compose up
```

# development
```bash
docker build -t ethernet .
docker run --net=host -it ethernet

# server
cd ./build/bin/
./server

# client
cd ./build/bin
./client
```
