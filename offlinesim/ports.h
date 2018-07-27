#ifndef PORTS_H
#define PORTS_H


class Sender{
public:
  double max_avail_bandwidth;
  double left_bandwidth;
  int sender_id;
  double data_sent;
  double completion_time;
  Sender(int sender_id, double max_avail_bandwidth);
};

class Receiver{
public:
  double max_avail_bandwidth;
  double left_bandwidth;
  int receiver_id;
  double data_sent;
  double completion_time;
  Receiver(int receiver_id, double max_avail_bandwidth);
};


#endif
