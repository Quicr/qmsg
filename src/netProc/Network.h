#pragma once
#include <map>
#include <queue>
#include <set>

#include <iostream>

#include <quicr/quicr_client.h>

struct QuicrMessageProcessor {
    virtual void on_quicr_message(const std::string& name, quicr::bytes&& message, std::uint64_t object_id) = 0;
};

struct QuicrMessageInfo {
    std::string name;
    std::uint64_t group_id;
    std::uint64_t object_id;
    quicr::bytes data;
};

struct QuicrDelegate: public quicr::QuicRClient::Delegate {

    virtual void on_data_arrived(const std::string& name,
                               quicr::bytes&& data,
                               std::uint64_t group_id,
                               std::uint64_t object_id) override {

      std::lock_guard<std::mutex> lock(queue_mutex);
      received_byte_queues[name].push(QuicrMessageInfo{name, group_id, object_id, data});
  }

    virtual void on_connection_close(const std::string& name) override{
        log(quicr::LogLevel::info, "[Delegate] Media Connection Closed: " + name);
        // trigger a resubscribe
    }

    virtual void log(quicr::LogLevel level, const std::string& message) override {
        std::clog <<  message << std::endl;
    }

private:
  std::mutex queue_mutex;
  std::map<std::string, std::queue<QuicrMessageInfo>> received_byte_queues;
};


enum struct EventSource {
    SecProc = 0,
    Network = 1
};


// Main class holding logic to participate
// in the Qmsg Flow
struct Network
{
  explicit Network(const std::string& server_ip, const uint16_t port);
  ~Network() = default;

  // public api
  void set_kphash_for_welcome(std::string&& team_id, std::string&& hash);

  void publish(uint32_t team_id, uint32_t channel_id, uint16_t device_id, quicr::bytes&& data);
  void subscribe_to_devices(uint32_t team_id, uint32_t channel_id, std::vector<uint16_t>&& devices);

  void unsubscribe_from_device(uint32_t team_id, uint32_t channel_id, uint16_t device_id);
  void subscribe_for_keypackage(uint32_t team_id, quicr::bytes&& kp_hash);
  
  // event handlers
  void handleDeviceInfo(const uint32_t team_id, const uint16_t device_id);
  void handleKeyPackageEvent(EventSource source, const uint32_t team_id, quicr::bytes&& key_package, quicr::bytes&& key_package_hash);
  void handleMLSWelcomeEvent(EventSource source, const uint32_t team_id, quicr::bytes&& welcome);
  void handleMLSCommitEvent(EventSource source, const uint32_t team_id, quicr::bytes&& commit);

private:

  void publish(std::string&& name, quicr::bytes&& data);
  void subscribe(std::vector<std::string>&& names);

  std::map<uint32_t, std::string> keypackage_hashes;
  std::map <std::string, bool> publisher_registration_status;
  std::set<std::string> subscribers;
  QuicrDelegate delegate;
  quicr::QuicRClient qr_client;
};