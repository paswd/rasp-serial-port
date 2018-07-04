#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <wiringSerial.h>
#include "zmq.h"

using namespace std;

const string SERVER_PORT_DEFAULT = "4040";
const size_t MESSAGE_SIZE = 2;

int main(void) {
	
	cout << "Server starting..." << endl;
		
	size_t server_id = 0;
	string str_server_id = "";
	//server_id = StringToUNum(str_server_id);
	//string server_id_str = UNumToString(server_id);
	string server_bind = "tcp://*:" + SERVER_PORT_DEFAULT;
	void *context = zmq_ctx_new();
	void *respond = zmq_socket(context, ZMQ_REP);
	zmq_bind(respond, server_bind.c_str());

	string base = "/dev/ttyACM";
	int fd = -1;
	for (int i = 0; fd == -1 && i < 10; i++) {
		char tmp = i + '0';
		string addr = base + tmp;
		cout << "Connecting " << addr << "... " endl;
		fd = serialOpen(addr.c_str(), 9600);
		if (fd != -1) {
			cout << "OK";
		} else {
			cout << "FAILED";
		}
		cout << endl;
	}
	if (fd == -1) {
		cout << "Arduino connection error" << endl;
		return 0;
	}
	char bf[MESSAGE_SIZE + 1];
	bf[MESSAGE_SIZE] = '\n';
	cout << "Server has been successfully started" << endl;
	cout << "Using port: 4040" << endl;

	while (true) {
		//char bf[MESSAGE_SIZE];
		zmq_msg_t request;
		zmq_msg_init(&request);
		zmq_msg_recv(&request, respond, 0);
		memcpy(&bf, zmq_msg_data(&request), MESSAGE_SIZE);
		cout << "Got command: `" << bf[0] << bf[1] << "`" << endl;

		serialPrintf(fd, bf); //Sending to Arduino

		//string cmd(bf);
		zmq_msg_close(&request);


		zmq_msg_t reply;
		zmq_msg_init_size(&reply, MESSAGE_SIZE);
		memcpy(zmq_msg_data(&reply), bf, MESSAGE_SIZE);
		zmq_msg_send(&reply, respond, 0);
		zmq_msg_close(&reply);
	}

	zmq_close(respond);
	zmq_ctx_destroy(context);

	return 0;
}
