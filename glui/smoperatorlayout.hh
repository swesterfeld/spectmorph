// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_OPERATOR_LAYOUT_HH
#define SPECTMORPH_OPERATOR_LAYOUT_HH

namespace SpectMorph
{

class OperatorLayout
{
  struct Row
  {
    int width = 0; // FIXED
    int space = 0; // FIXED
    int height = 0;

    Widget *widget1 = nullptr;
    Widget *widget2 = nullptr;
    Widget *widget3 = nullptr;
  };
  std::vector<Row> rows;
public:

  void
  add_fixed (int width, int height, Widget *widget, int space = 1)
  {
    Row row;

    row.width = width;
    row.height = height;
    row.space = space;
    row.widget1 = widget;

    rows.push_back (row);
  }
  void
  add_row (int height, Widget *widget1 = nullptr, Widget *widget2 = nullptr, Widget *widget3 = nullptr)
  {
    Row row;
    row.height = height;
    row.widget1 = widget1;
    row.widget2 = widget2;
    row.widget3 = widget3;
    rows.push_back (row);
  }
  uint
  activate()
  {
    FixedGrid grid;

    int yoffset = 0;
    int width[3] = { 9, 20, 8 };

    for (auto row : rows)
      {
        bool visible = false;

        if (row.widget1)
          visible = visible || row.widget1->visible();
        if (row.widget2)
          visible = visible || row.widget2->visible();
        if (row.widget3)
          visible = visible || row.widget3->visible();

        if (!visible)
          {
            // skip widget(s)
          }
        else if (row.width != 0) // FIXED SIZE WIDGET
          {
            int w = width[0] + 1 + width[1] + 1 + width[2];
            yoffset += row.space;
            grid.add_widget (row.widget1, (w - row.width) / 2, yoffset, row.width, row.height);
            yoffset += row.height + row.space;
          }
        else
          {
            int start = 0, w = 0;
            if (row.widget1)
              {
                w = width[0];
                if (!row.widget2) // if next widgets are missing: grow
                  {
                    w += 1 + width[1];
                    if (!row.widget3)
                      w += 1 + width[2];
                  }
                grid.add_widget (row.widget1, start, yoffset, w, row.height);
                start += w + 1;
              }

            if (row.widget2)
              {
                w = width[1];
                if (!row.widget3)
                  w += 1 + width[2];

                grid.add_widget (row.widget2, start, yoffset, w, row.height);
                start += w + 1;
              }

            if (row.widget3)
              {
                w = width[2];
                grid.add_widget (row.widget3, start, yoffset, w, row.height);
              }

            yoffset += row.height;
          }
      }
    return yoffset;
  }
};

}

#endif
