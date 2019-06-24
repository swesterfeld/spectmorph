// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MAIN_HH
#define SPECTMORPH_MAIN_HH

namespace SpectMorph
{

void sm_init_plugin();
void sm_cleanup_plugin();
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
