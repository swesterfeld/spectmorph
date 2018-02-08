// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smsignal.hh"

using namespace SpectMorph;

using std::string;

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
}
