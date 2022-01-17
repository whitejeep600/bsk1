openssl genrsa -out ca.key 2048
answers="/C=PL/ST=Mazowieckie/L=Warsaw/O=BSK-CA-2021"
openssl req -new -x509 -days 380 -key ca.key -out ca.crt -subj $answers
openssl genrsa -out bsk.key 2048
answers2="/C=PL/ST=Mazowieckie/L=Warsaw/CN=am418402.zadanie1.bsk"
openssl req -new -key bsk.key -out bsk.csr -subj $answers2
openssl x509 -req -in bsk.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out bsk.crt
mv bsk.crt /usr/local/share/ca-certificates/
