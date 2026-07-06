#ifndef NETWORK_ROUTING_SIMULATOR_SIMULATOR_H
#define NETWORK_ROUTING_SIMULATOR_SIMULATOR_H

#include <string>
#include <vector>

#include "Network.h"
#include "Packet.h"

/**
 * Simulator
 * ---------
 * Drives the interactive command-line interface. Simulator owns the
 * Network instance and tracks runtime statistics (packets sent/delivered,
 * average latency, topology changes). It is responsible for parsing user
 * commands and dispatching them to the appropriate Network operations,
 * printing user-facing feedback along the way.
 */
class Simulator {
public:
    Simulator();

    // Runs the read-eval-print loop until the user issues EXIT/QUIT or EOF.
    void run();

private:
    Network network_;

    // Runtime statistics
    unsigned long long packetsSent_ = 0;
    unsigned long long packetsDelivered_ = 0;
    double totalDeliveredLatency_ = 0.0;

    // ---- Command handlers -------------------------------------------------
    void handleCommand(const std::vector<std::string>& tokens);

    void cmdAdd(const std::vector<std::string>& args);
    void cmdRemove(const std::vector<std::string>& args);
    void cmdLink(const std::vector<std::string>& args);
    void cmdUnlink(const std::vector<std::string>& args);
    void cmdUpdateLink(const std::vector<std::string>& args);
    void cmdPrint();
    void cmdPath(const std::vector<std::string>& args);
    void cmdTable(const std::vector<std::string>& args);
    void cmdSend(const std::vector<std::string>& args);
    void cmdFail(const std::vector<std::string>& args);
    void cmdRestore(const std::vector<std::string>& args);
    void cmdLoad(const std::vector<std::string>& args);
    void cmdSave(const std::vector<std::string>& args);
    void cmdStats() const;
    void cmdHelp() const;

    // ---- Helpers ------------------------------------------------------------
    Packet sendPacket(const std::string& source, const std::string& destination);
    static std::vector<std::string> tokenize(const std::string& line);
};

#endif // NETWORK_ROUTING_SIMULATOR_SIMULATOR_H
