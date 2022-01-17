docker build --rm -t bsk_image .
docker container run -p 2137:2137 -t -d --name bsk_container bsk_image
docker exec -it bsk_container bash -c /permissions_setup.sh

