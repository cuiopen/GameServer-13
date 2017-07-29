#include <clientSession.h>
#include <gameSession.h>
#include <global.h>

clientSession::clientSession(clientServer* server, const khaki::TcpClientPtr& conn):
    server_(server), conn_(conn) {
    conn_->setReadCallback(std::bind(&clientSession::OnMessage, 
                        this, std::placeholders::_1));
    RegisterCmd();
}

clientSession::~clientSession() {

}

void clientSession::RegisterCmd() {
    command_[cs::ProtoID::ID_C2S_Ping] = std::bind(&clientSession::HandlerPing, this, std::placeholders::_1);
    command_[cs::ProtoID::ID_C2S_Login] = std::bind(&clientSession::HandlerLogin, this, std::placeholders::_1);
}

void clientSession::DispatcherCmd(struct PACKET& msg) {
    if ( command_.find(msg.cmd) != command_.end() ) {
        command_[msg.cmd](msg);
    } else if (status_ == E_CLIENT_RUNNING) {
        switch ( msg.cmd ) {
            case cs::ProtoID::ID_C2S_Login:
                UnAuthSendToServer(msg);
                break;
        }
    } else {
        log4cppDebug(khaki::logger, "error proto : %d", msg.cmd);
    }
}

void clientSession::OnMessage(const khaki::TcpClientPtr& con) {
    khaki::Buffer& buf = con->getReadBuf();
    while( buf.size() > 0 ) {
        if (!buf.checkInt32()) break;
        struct PACKET pkt = Decode(buf);
        DispatcherCmd(pkt);
    }
}   

void clientSession::SendToServer(struct PACKET& msg) {
    std::shared_ptr<gameSession> gsp = gameSession_.lock();
    if ( gsp ) {
        std::string str = Encode(msg);
        gsp->SendToGameServer(str);
    } else {
        log4cppDebug(khaki::logger, "Auth, gameserver not exist error");
    }
}

void clientSession::UnAuthSendToServer(struct PACKET& msg) {
    std::shared_ptr<gameSession> gsp = g_gServer->GetGameSessionBySid(msg.sid);
    if ( gsp ) {
        std::string str = Encode(msg);
        gsp->SendToGameServer(str);
    } else {
         log4cppDebug(khaki::logger, "UnAuth, gameserver not exist");
    }
}

bool clientSession::HandlerPing(struct PACKET& pkt) {
    cs::C2S_Ping recv;
    if ( !recv.ParseFromString(pkt.msg) )
    {
        log4cppDebug(khaki::logger, "proto parse error : %d", pkt.cmd);
        return false;
    }
    log4cppDebug(khaki::logger, "HandlerPing uid : %d, sid : %d, cmd : %d", pkt.uid, pkt.sid, pkt.cmd);
}

bool clientSession::HandlerLogin(struct PACKET& pkt) {
    cs::C2S_Login recv;
    if ( !recv.ParseFromString(pkt.msg) )
    {
        log4cppDebug(khaki::logger, "proto parse error : %d", pkt.cmd);
        return false;
    }

    gameSession_ = g_gServer->GetGameSessionBySid(pkt.sid);
    SendToServer(pkt);
}

bool clientSession::HandlerDirtyPacket(struct PACKET& str) {

}