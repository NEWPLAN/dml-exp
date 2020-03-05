#ifndef RPCPROTHEAD_H
#define RPCPROTHEAD_H
#include <stdint.h>
#include <string>
using namespace std;

#define FRPC_MAGIC 0xe3

#define EMsgCmdType_Req         0
#define EMsgCmdType_Rsp         1
#define EMsgCmdType_Notify      2
#define ERPCMSGTYPE_SYSRSP      3
#define ERPCMSGTYPE_DATAFRAG    4
#define EMsgCmdType_ConnLost    -2

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint8_t cmdtype;
    uint8_t flag;
    uint16_t headcrc16;
    uint16_t datacrc;
    uint16_t seq;
    uint16_t timeout;
    uint32_t datalen;
}protoHead;

#define BREADNETWIDTH 0x08
#define EXTRADATA   0x04
#define ENCRYPTED   0x02
#define COMPRESSED  0x01

class RpcProtHead
{

public:
    RpcProtHead();
    int HeadSize()
    {
        return sizeof(mHead);//16
    }

    void initWithRecv(string &parm);

    void setEncrpted(bool paramBoolean);
    void setCompressed(bool paramBoolean);
    void setExtradata(string &paramArrayOfByte);
    void setBreadNetWidth(bool paramBoolean);
    void packHead(char *paramPackData);

    bool hasExtraData();
    bool isCompressed();
    bool isEncrpted();

    string extradata;
    protoHead mHead;
    string data;
private:
    void headdump();

};

#endif // RPCPROTHEAD_H
