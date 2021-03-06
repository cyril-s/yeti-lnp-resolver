#pragma once

#include <string>
using std::string;

#include <pqxx/pqxx>

#define RESOLVE_EXCEPTION_DRIVER 7
#define RESOLVE_EXCEPTON_REASON_UNEXPECTED "unexpected reply"

struct resolve_exception {
	int code;
	std::string reason;
	resolve_exception(int code, std::string reason):
		code(code), reason(reason) {}
};

enum drivers_t {
	RESOLVER_DRIVER_SIP = 1,
    RESOLVER_DRIVER_HTTP_THINQ,
    RESOLVER_DRIVER_MHASH_CSV
};
static const char *driver_id2name(int id)
{
	static const char *sip = "SIP/301-302";
	static const char *http_thinq = "REST/ThinQ";
    static const char *mhash_csv = "MHASH/CSV";
	static const char *unknown = "unknown";
	switch(id){
		case RESOLVER_DRIVER_SIP: return sip; break;
		case RESOLVER_DRIVER_HTTP_THINQ: return http_thinq; break;
        case RESOLVER_DRIVER_MHASH_CSV: return mhash_csv; break;
		default: return unknown;
	}
}

class resolver_driver {
  public:
	struct driver_cfg {
		int database_id,driver_id,port;
		unsigned int timeout;
		string name,host;
		string thinq_token,thinq_username;
        string data_path;
		driver_cfg(const pqxx::result::tuple &r);
	};
    struct result {
        string lrn;  //local routing number
        string tag;  //optional tag
        string raw_data; //raw data special for some types of drivers
    };
  protected:
	driver_cfg _cfg;
  public:
	resolver_driver(const driver_cfg &cfg);
	virtual ~resolver_driver();
	virtual void on_stop() {}
	virtual void launch() {}

    virtual void resolve(const string &in, struct result &out) = 0;

	virtual void show_info(int map_id);
};
