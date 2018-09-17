#include "Akka.h"
#include <MqttBridge.h>
#include <etl/endianness.h>

using namespace etl;

Log logger(1024);

#define MAX_MESSAGES 100000000

// const static MsgClass PING =Uid::hash("PING");
const static MsgClass PONG("PONG");
const static MsgClass PING("PING");

ActorSystem actorSystem(Sys::hostname());

class Echo : public AbstractActor {
    Str str;

  public:
    Echo(va_list args) : str(80) {}
    ~Echo() {}

    void preStart() {}
    Receive& createReceive() {
        return receiveBuilder()
            .match(PING,
                   [this](Envelope& msg) {
                       uint32_t counter;
                       msg.scanf("uS", &counter, &str);
//                       sender().tell(self(), PONG, "us", counter, str.c_str());
                       sender().tell(self(), PONG,msg.id, "us", counter, str.c_str());
                   })
            .match(("ikke"),
                   [this](Envelope& msg) {
                       INFO(" message received %s:%s:%s in %s",
                            msg.sender.path(), msg.receiver.path(),
                            msg.msgClass.label(), context().self().path());
                   })
            .build();
    }
};
//______________________________________________________________
//

class Sender : public AbstractActor {
    uint64_t startTime;
    Str str;
    ActorRef echo;
    ActorRef anchorSystem;

  public:
    Sender(va_list args) : startTime(0), str(80) {}
    ~Sender() {}

    void preStart() {
        echo = actorSystem.actorOf<Echo>("echo");
        timers().startPeriodicTimer("PERIODIC_TIMER_1", TimerExpired, 5000);
        anchorSystem = actorSystem.actorFor("anchor/system");
    }

    void finished() {
        float delta = Sys::millis() - startTime;
        INFO(" '%s' done in %f msec %s ", self().path(), delta, str.c_str());
        INFO(" %f msg/sec ", MAX_MESSAGES * 1000.0 / delta);
    }

    Receive& createReceive() {
        return receiveBuilder()
            .match(PONG,
                   [this](Envelope& msg) {
                       //			INFO(" PONG received");
                       uint32_t counter;
                       msg.scanf("uS", &counter, &str);
                       if (counter == 0) {
                           startTime = Sys::millis();
                       } else if (counter == MAX_MESSAGES) {
                           finished();
                       }
                       if (counter < MAX_MESSAGES)
                           msg.sender.tell(self(), PING, "us", ++counter,
                                           "Hi ");
                   })
            .match(TimerExpired,
                   [this](Envelope& msg) {
                       UidType key("");
                       int k;
                       msg.scanf("i", &k);
                       key = k;
                       INFO(" timer expired ! %s ", key.label());
                       //                       timers().cancel("STARTER");
                       echo.tell(self(), PING, "us", 0, "hi!");
                       anchorSystem.tell(
                           self(), "reset", "s",
                           "The quick brown fox jumps over the lazy dog");
                   })
            .build();
    }

    void handle(Envelope& msg) {
        if (msg.msgClass == PONG) {
            uint32_t counter;
            msg.scanf("uS", &counter, &str);
            if (counter == 0) {
                startTime = Sys::millis();
            } else if (counter == MAX_MESSAGES) {
                finished();
            }
            if (counter < MAX_MESSAGES)
                msg.sender.tell(self(), PING, "us", ++counter, "Hi ");
        }
    }
};

uint32_t millisleep(uint32_t msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec - ts.tv_sec * 1000) * 1000000;
    int erc = nanosleep(&ts, NULL);
    return erc;
};

Mailbox defaultMailbox("default", 20000, 1000);
Mailbox coRoutineMailbox("coRoutine", 20000, 1000);
Mailbox remoteMailbox("$remote", 20000, 1000);
MessageDispatcher defaultDispatcher;

int main() {
    Sys::init();

    INFO(" starting microAkka test ");
    if ( etl::endianness::value()==etl::endian::little ) {
        INFO(" little endian ");
    }
    if ( etl::endianness::value()==etl::endian::big ) {
        INFO(" big endian ");
    }
    //    ActorRef echo = actorSystem.actorOf<Echo>("echo");
    ActorRef sender = actorSystem.actorOf<Sender>("sender");
    ActorRef mqttBridge = actorSystem.actorOf<MqttBridge>(
        "mqttBridge", "tcp://test.mosquitto.org:1883");

    defaultDispatcher.attach(defaultMailbox);
    defaultDispatcher.attach(remoteMailbox);
    defaultDispatcher.attach(*ActorContext::context(sender));
    defaultDispatcher.unhandled(ActorContext::context(mqttBridge));

    while (true) {
        defaultDispatcher.execute();
        millisleep(10);
    };
}
