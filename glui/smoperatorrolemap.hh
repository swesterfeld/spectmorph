// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_OPERATOR_ROLE_MAP_HH
#define SPECTMORPH_OPERATOR_ROLE_MAP_HH

namespace SpectMorph
{

class OperatorRoleMap
{
  std::map<MorphOperator *, int> op_map;

  void
  crawl (MorphOperator *op, int role)
  {
    int& value = op_map[op];
    if (value == 0)
      {
        value = role;
        for (auto dep_op : op->dependencies())
          {
            if (dep_op)
              crawl (dep_op, role + 1);
          }
      }
  }
public:
  void
  rebuild (MorphPlan *plan)
  {
    op_map.clear();
    for (auto op : plan->operators())
      {
        if (strcmp (op->type(), "SpectMorph::MorphOutput") == 0)
          {
            crawl (op, 1);
          }
      }
  }

  int
  get (MorphOperator *op)
  {
    return op_map[op];
  }
};

}

#endif
