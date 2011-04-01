/* MicroConf - minimal configuration framework
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <string>
#include <vector>

#include <stdio.h>

namespace SpectMorph
{

class MicroConf
{
private:
  FILE                    *cfg_file;
  std::string              current_line;
  int                      current_no;
  std::string              current_file;
  std::vector<std::string> tokens;
  bool                     tokenizer_error;

  bool convert (const std::string& token, int& arg);
  bool convert (const std::string& token, double& arg);
  bool convert (const std::string& token, std::string& arg);

  bool tokenize();
public:
  MicroConf (const std::string& filename);
  ~MicroConf();

  bool open_ok();
  bool next();
  std::string line();
  void die_if_unknown();

  template<class T1>
  bool command (const std::string& cmd, T1& arg1)
  {
    if (tokenizer_error || tokens.size() != 2 || cmd != tokens[0])
      return false;
    return convert (tokens[1], arg1);
  }
  template<class T1, class T2>
  bool command (const std::string& cmd, T1& arg1, T2& arg2)
  {
    if (tokenizer_error || tokens.size() != 3 || cmd != tokens[0])
      return false;
    return convert (tokens[1], arg1) && convert (tokens[2], arg2);
  }
  template<class T1, class T2, class T3>
  bool command (const std::string& cmd, T1& arg1, T2& arg2, T3& arg3)
  {
    if (tokenizer_error || tokens.size() != 4 || cmd != tokens[0])
      return false;
    return convert (tokens[1], arg1) && convert (tokens[2], arg2) && convert (tokens[3], arg3);
  }
  template<class T1, class T2, class T3, class T4>
  bool command (const std::string& cmd, T1& arg1, T2& arg2, T3& arg3, T4& arg4)
  {
    if (tokenizer_error || tokens.size() != 5 || cmd != tokens[0])
      return false;
    return convert (tokens[1], arg1) && convert (tokens[2], arg2) && convert (tokens[3], arg3)
      &&   convert (tokens[4], arg4);
  }
  template<class T1, class T2, class T3, class T4, class T5>
  bool command (const std::string& cmd, T1& arg1, T2& arg2, T3& arg3, T4& arg4, T5& arg5)
  {
    if (tokenizer_error || tokens.size() != 6 || cmd != tokens[0])
      return false;
    return convert (tokens[1], arg1) && convert (tokens[2], arg2) && convert (tokens[3], arg3)
      &&   convert (tokens[4], arg4) && convert (tokens[5], arg5);
  }
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  bool command (const std::string& cmd, T1& arg1, T2& arg2, T3& arg3, T4& arg4, T5& arg5, T6& arg6)
  {
    if (tokenizer_error || tokens.size() != 7 || cmd != tokens[0])
      return false;
    return convert (tokens[1], arg1) && convert (tokens[2], arg2) && convert (tokens[3], arg3)
      &&   convert (tokens[4], arg4) && convert (tokens[5], arg5) && convert (tokens[6], arg6);
  }
};

}
