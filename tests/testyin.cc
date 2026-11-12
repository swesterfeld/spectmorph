// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <vector>
#include <cstdio>
#include <cstdlib>

using std::vector;

namespace
{

class QInterpolator
{
  double a, b, c;

public:
  QInterpolator (double y1, double y2, double y3)
  {
    a = (y1 + y3 - 2*y2) / 2;
    b = (y3 - y1) / 2;
    c = y2;
  }
  double
  eval (double x)
  {
    return a * x * x + b * x + c;
  }
  double
  x_max()
  {
    return -b / (2 * a);
  }
};

}


int
main()
{
  vector<float> f;
  char buffer[1024];
  while (fgets (buffer, 1024, stdin))
    {
      f.push_back (atof (buffer));
    }

  size_t win = 1500;
  size_t hop = win / 4;
  for (size_t i = 0; i + win + win < f.size(); i += hop)
    {
      float D[win];
      for (size_t delay = 0; delay < win; delay++)
        {
          D[delay] = 0;
          for (size_t j = 0; j < win; j++)
            {
              float d = f[i + j] - f[i + j + delay];
              D[delay] += d * d;
            }
        }
      float D1[win];
      D1[0] = 1;
      for (size_t delay = 1; delay < win; delay++)
        {
          float avg = 0;
          for (size_t j = 1; j <= delay; j++)
            avg += D[j];
          D1[delay] = D[delay] / (avg / delay);
        }
      float pitch = 0, pitch2 = 0;
      for (size_t dmin = 1; dmin < win - 1; dmin++)
        {
          if (D1[dmin] < D1[dmin - 1] && D1[dmin] < D1[dmin + 1] && D1[dmin] < 0.3)
            {
              pitch = 48000.0 / dmin;
              QInterpolator interp (-D[dmin - 1], -D[dmin], -D[dmin + 1]);
              pitch2 = 48000 / (dmin + interp.x_max());
              //printf ("%f\n", interp.x_max());
              break;
            }
        }
      printf ("%zd %f %f\n", i, pitch, pitch2);
    }
}
