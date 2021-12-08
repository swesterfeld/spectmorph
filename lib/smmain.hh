// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MAIN_HH
#define SPECTMORPH_MAIN_HH

namespace SpectMorph
{

void sm_plugin_init();
void sm_plugin_cleanup();
bool sm_init_done();
bool sm_sse();
void sm_enable_sse (bool sse);

class InstEncCache;
class WavSetRepo;

namespace Global
{
  InstEncCache *inst_enc_cache();
  WavSetRepo   *wav_set_repo();
}

class Main
{
public:
  Main (int *argc_p, char ***argv_p);
  ~Main();
};

}

#endif
