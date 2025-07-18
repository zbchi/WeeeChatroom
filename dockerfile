FROM ubuntu:24.04

ADD ./build/server/chat_server /home/chat_server

RUN apt update && apt install -y \
			libcurl4-openssl-dev \
	 		libmysqlclient21

WORKDIR /home
CMD ["./chat_server"]

