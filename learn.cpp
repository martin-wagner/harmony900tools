//https://chatgpt.com/c/69b6d342-05b4-832a-a0fb-15bedba19922

#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <cmath>

using namespace std;

struct Frame
{
    uint8_t sync;
    uint8_t cmd;
    uint8_t type;
    uint8_t len;
    vector<uint8_t> payload;
    uint8_t checksum;
};

uint8_t checksum(const vector<uint8_t>& f)
{
    uint8_t s=0;
    for(size_t i=0;i<f.size()-1;i++)
        s+=f[i];
    return s;
}

void dumpHex(const vector<uint8_t>& data)
{
    for(auto b:data)
        cout<<hex<<setw(2)<<setfill('0')<<(int)b<<" ";
    cout<<dec<<endl;
}

bool readFrame(int sock, vector<uint8_t>& frame)
{
    uint8_t hdr[4];

    if(recv(sock,hdr,4,MSG_WAITALL)!=4)
        return false;

    frame.assign(hdr,hdr+4);

    uint8_t buf[32];

    int r=recv(sock,buf,sizeof(buf),MSG_DONTWAIT);

    if(r>0)
        frame.insert(frame.end(),buf,buf+r);

    return true;
}

vector<uint16_t> parsePayload(const vector<uint8_t>& p)
{
    vector<uint16_t> d;

    for(size_t i=0;i+2<p.size();i++)
    {
        if(p[i]==0x02)
        {
            uint16_t v=p[i+1]|(p[i+2]<<8);
            d.push_back(v);
            i+=2;
        }
    }

    return d;
}

bool isIdle(const vector<uint16_t>& d)
{
    for(auto v:d)
        if(v>400 && v<30000)
            return false;
    return true;
}

vector<int> decodeManchester(const vector<uint16_t>& t)
{
    vector<int> bits;

    double half=889;

    for(auto v:t)
    {
        if(fabs(v-half)<300)
            bits.push_back(1);
        else if(fabs(v-half*2)<500)
        {
            bits.push_back(1);
            bits.push_back(1);
        }
    }

    return bits;
}

void decodeRC5(const vector<uint16_t>& timings)
{
    cout<<"---- RC5 decode attempt ----"<<endl;

    auto bits=decodeManchester(timings);

    if(bits.size()<28)
    {
        cout<<"not enough bits"<<endl;
        return;
    }

    vector<int> rc5;

    for(size_t i=0;i<bits.size();i+=2)
        rc5.push_back(bits[i]);

    if(rc5.size()<14)
    {
        cout<<"invalid frame"<<endl;
        return;
    }

    int toggle=rc5[2];

    int addr=0;
    for(int i=3;i<8;i++)
        addr=(addr<<1)|rc5[i];

    int cmd=0;
    for(int i=8;i<14;i++)
        cmd=(cmd<<1)|rc5[i];

    cout<<"toggle: "<<toggle<<endl;
    cout<<"device: "<<addr<<endl;
    cout<<"command: "<<cmd<<endl;
}

int main(int argc,char**argv)
{
    if(argc!=3)
    {
        cout<<"usage: ircap ip port"<<endl;
        return 0;
    }

    int s=socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in a{};
    a.sin_family=AF_INET;
    a.sin_port=htons(atoi(argv[2]));
    if (inet_pton(AF_INET,argv[1],&a.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(s);
        return 1;
    }

    if (connect(s,(sockaddr*)&a,sizeof(a)) < 0) {
        perror("Connection failed");
        close(s);
        return 1;
    }

    cout<<"connected"<<endl;

    uint8_t start[]={0x20,0xA1,0x80,0x01,0x01,0x00};
    send(s,start,sizeof(start),0);

    cout<<"capture started"<<endl;

    sleep(1);

    cout<<"press remote"<<endl;

    vector<uint16_t> capture;

    while(true)
    {
        uint8_t poll[]={0x20,0xA2,0x80,0x00};
        send(s,poll,sizeof(poll),0);

        vector<uint8_t> frame;

        if(!readFrame(s,frame))
            break;

        cout<<"frame: ";
        dumpHex(frame);

        if(frame.size()<6)
            continue;

        vector<uint8_t> payload(frame.begin()+4,frame.end()-1);

        cout<<"payload: ";
        dumpHex(payload);

        auto d=parsePayload(payload);

        if(isIdle(d))
        {
            cout<<"idle detected -> end capture"<<endl;
            break;
        }

        capture.insert(capture.end(),d.begin(),d.end());
    }

    cout<<"---- reconstructed timings ----"<<endl;

    for(auto v:capture)
        cout<<v<<" ";

    cout<<endl;

    decodeRC5(capture);

    uint8_t stop[]={0x20,0xA4,0x80,0x00};
    send(s,stop,sizeof(stop),0);

    close(s);
}
