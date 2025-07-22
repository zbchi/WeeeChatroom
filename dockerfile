FROM ubuntu:22.04

ADD ./build/server/chat_server /home/chat_server

RUN apt update && apt install -y \
			libcurl4-openssl-dev \
	 		libmysqlclient21 \
			uuid-dev

WORKDIR /home
CMD ["./chat_server"]

