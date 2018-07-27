#include "ports.h"

Sender::Sender(int sender_id, double max_avail_bandwidth){
  this->max_avail_bandwidth = max_avail_bandwidth;
  this->left_bandwidth = max_avail_bandwidth;
  this->sender_id = sender_id;
  this->data_sent = 0;
  this->completion_time = 0;
}

Receiver::Receiver(int receiver_id, double max_avail_bandwidth){
  this->max_avail_bandwidth = max_avail_bandwidth;
  this->left_bandwidth = max_avail_bandwidth;
  this->receiver_id = receiver_id;
  this->data_sent = 0;
  this->completion_time = 0;
}
