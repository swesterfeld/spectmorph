// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smsignal.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

Signal<> signal_simple;
Signal<int> signal_int;
Signal<std::string, double> signal_sd;

struct Receiver : SignalReceiver
{
  string message;
  Receiver (const string& message) :
    message (message)
  {
    connect (signal_simple, [&]() { simple_slot(); });
    connect (signal_int,    [&](int i) { printf ("%s::%d\n", message.c_str(), i); });
  }
  void
  simple_slot()
  {
    printf ("%s\n", message.c_str());
  }
};

struct ModifyInCallback : public SignalReceiver
{
  Signal<int> signal_add_connections;
  Signal<>    signal_del_connections;
  vector<uint64> del_ids;
  int conn_count = 0;

  void
  add_connection()
  {
    connect (signal_add_connections, [&](int count) {
      while (conn_count < count)
        add_connection();
    });
    conn_count++;
  }
  ModifyInCallback()
  {
    while (conn_count < 10)
      {
        add_connection();
      }
    printf ("add connections test\n");
    signal_add_connections (1000);

    for (int i = 0; i < 100; i++)
      {
        del_ids.push_back (connect (signal_del_connections, [this]() {
          for (auto id : del_ids)
            signal_del_connections.disconnect (id);
        }));
      }
    printf ("del connections test\n");
    signal_del_connections();
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  printf ("---- expect: nothing\n");
  signal_simple();
  printf ("---- end ----\n\n");

  Receiver *receiver = new Receiver ("receiver 1");
  printf ("---- expect: something\n");
  signal_simple();
  printf ("---- end ----\n\n");

  delete receiver;
  printf ("---- expect: nothing\n");
  signal_simple();
  printf ("---- end ----\n\n");

  Receiver *int_r = new Receiver ("int_r");
  printf ("---- expect: int 23\n");
  signal_int (23);
  printf ("---- end ----\n\n");
  delete int_r;

  printf ("---- expect: nothing\n");
  signal_int (23);
  printf ("---- end ----\n\n");

  printf ("---- expect: int 3\n");
  {
    struct StackR : public SignalReceiver
    {
      static void
      slot (const std::string& s, double d)
      {
        printf ("%s ## %f\n", s.c_str(), d);
      }
    } stack_r;
    stack_r.connect (signal_sd, StackR::slot);
    signal_sd ("foo", 3);
  }
  printf ("---- end ----\n\n");

  printf ("---- expect: nothing\n");
  signal_sd ("foo", 3);
  printf ("---- end ----\n\n");

  Receiver *r = new Receiver ("junk");
  {
    Signal<> signal_temporary;
    r->connect (signal_temporary, [] () { printf ("signal_temporary\n"); });
    signal_temporary();
  }
  delete r; // signal should be dead by now

  ModifyInCallback mic;
}
