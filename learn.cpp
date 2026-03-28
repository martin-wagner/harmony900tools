//https://chatgpt.com/c/69b6d342-05b4-832a-a0fb-15bedba19922

#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstring>

using namespace std;

void hexDump(const string& label,const vector<uint8_t>& data)
{
    cout<<label<<" ("<<data.size()<<"): ";

    for(auto b:data)
        cout<<hex<<setw(2)<<setfill('0')<<(int)b<<" ";

    cout<<dec<<endl;
}

bool readFrame(int sock,vector<uint8_t>& frame)
{
    uint8_t hdr[4];

    if(recv(sock,hdr,4,MSG_WAITALL)!=4)
        return false;

    frame.assign(hdr,hdr+4);

    uint8_t buf[256];

    int r=recv(sock,buf,sizeof(buf),MSG_DONTWAIT);

    if(r>0)
        frame.insert(frame.end(),buf,buf+r);

    return true;
}

uint8_t checksum(const vector<uint8_t>& f)
{
    uint8_t s=0;

    for(size_t i=0;i<f.size()-1;i++)
        s+=f[i];

    return s;
}

vector<uint16_t> extractDurations(const vector<uint8_t>& payload)
{
    vector<uint16_t> d;

    for(size_t i=0;i+1<payload.size();i+=2)
    {
        uint16_t v=payload[i]|(payload[i+1]<<8);
        d.push_back(v);
    }

    return d;
}

void decodeRC5(const vector<uint16_t>& d)
{
    cout<<"attempt RC5 decode"<<endl;

    if(d.size()<20)
    {
        cout<<"too few pulses for RC5"<<endl;
        return;
    }

    double avg=0;

    for(auto v:d)
        avg+=v;

    avg/=d.size();

    cout<<"avg duration "<<avg<<" us"<<endl;
}

int main(int argc,char*argv[])
{
    if(argc!=3)
    {
        cout<<"usage: prog ip port"<<endl;
        return 0;
    }

    int s=socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in a{};
    a.sin_family=AF_INET;
    a.sin_port=htons(atoi(argv[2]));
    inet_pton(AF_INET,argv[1],&a.sin_addr);

    connect(s,(sockaddr*)&a,sizeof(a));

    cout<<"connected"<<endl;

    uint8_t start[]={0x20,0xA1,0x80,0x01,0x01,0x00};

    send(s,start,sizeof(start),0);

    cout<<"sent start"<<endl;

    sleep(1);

    cout<<"press remote"<<endl;

    vector<uint8_t> fullPayload;

    for(int i=0;i<40;i++)
    {
        uint8_t poll[]={0x20,0xA2,0x80,0x00};

        send(s,poll,sizeof(poll),0);

        vector<uint8_t> frame;

        if(!readFrame(s,frame))
            break;

        hexDump("frame",frame);

        if(frame.size()<5)
            continue;

        uint8_t cmd=frame[1];

        if(cmd!=0xA2)
            continue;

        vector<uint8_t> payload(frame.begin()+4,frame.end()-1);

        hexDump("payload",payload);

        fullPayload.insert(fullPayload.end(),payload.begin(),payload.end());

        usleep(200000);
    }

    hexDump("full payload",fullPayload);

    auto durations=extractDurations(fullPayload);

    cout<<"durations:"<<endl;

    for(auto v:durations)
        cout<<v<<" ";

    cout<<endl;

    decodeRC5(durations);

    uint8_t stop[]={0x20,0xA4,0x80,0x00};

    send(s,stop,sizeof(stop),0);

    close(s);
}
