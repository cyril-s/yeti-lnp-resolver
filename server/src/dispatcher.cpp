#include "dispatcher.h"
#include "resolver.h"
#include "cfg.h"

#define PDU_HDR_SIZE 2

_dispatcher::_dispatcher():
	_stop(false)
{
}

void _dispatcher::loop()
{
	s = nn_socket(AF_SP,NN_REP);
	if(s < 0){
		throw std::string("nn_socket() = %d",s);
	}
	int ret = nn_bind(s, cfg.bind_url);
	if(ret < 0){
		err("can't bind to url '%s': %d(%s)",cfg.bind_url,
			 errno,nn_strerror(errno));
		throw std::string("can't bind to url");
	}

	info("listen on %s",cfg.bind_url);
	while(!_stop){
		char *msg = NULL;
		int l = nn_recv(s, &msg, NN_MSG, 0);
		if(l < 0){
			if(errno==EINTR) continue;
			dbg("nn_recv() = %d, errno = %d(%s)",l,errno,nn_strerror(errno));
			//!TODO: handle timeout, etc
			continue;
		}
		process_peer(msg,l);
		nn_freemsg(msg);
	}
}

int _dispatcher::process_peer(char *msg, int len)
{
	//dbg_func("msg[%p] len = %d",msg,len);
	/*for(int i = 0; i<len; i++){
		dbg("0x%02x: 0x%02x",i,msg[i]&0xff);
	}*/

	char *reply = NULL;
	int reply_len = 0;

	try {
		create_reply(reply,reply_len,msg,len);
	} catch(std::string &e){
		err("got string exception: %s",e.c_str());
		create_error_reply(reply,reply_len,255,std::string("Internal Error"));
	} catch(resolve_exception &e){
		err("got resolve exception: %d <%s>",e.code,e.reason.c_str());
		create_error_reply(reply,reply_len,e.code,e.reason);
	}

	int l = nn_send(s, reply, reply_len, 0);
	delete[] reply;
	if(l!=reply_len){
		err("nn_send(): %d, while msg to send size was %d",l,reply_len);
	}
	return 0;
}

void _dispatcher::create_error_reply(char *&msg, int &len,
									 int code, std::string description)
{
	int l = description.size();
	len = l+PDU_HDR_SIZE;
	msg = new char[len];
	msg[0] = code;
	msg[1] = l;
	memcpy(msg+PDU_HDR_SIZE,description.c_str(),l);
}

void _dispatcher::create_reply(char *&msg, int &len, const char *req, int req_len)
{
	std::string number;

	if(req_len < PDU_HDR_SIZE){
		throw resolve_exception(1,"request too small");
	}

	int data_len = req[1];

	if(data_len > req_len - PDU_HDR_SIZE){
		throw resolve_exception(1,"invalid data length");
	}
	int database_id = req[0];

	std::string lnp(req+2,req_len);

	dbg("process request: database: %d, number: %s",
		database_id,lnp.c_str());

	resolver::instance()->resolve(database_id,lnp,number);

	dbg("number = %s",number.c_str());

	int l = number.size();
	len = l+PDU_HDR_SIZE;
	msg = new char[len];
	msg[0]  = 0; //successfull code
	msg[1] = l;
	memcpy(msg+PDU_HDR_SIZE,number.c_str(),l);
}