// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smutils.hh"
#include "smmain.hh"

using namespace SpectMorph;

using std::string;

string get (InstallDir d)   { return sm_get_install_dir (d); }
string get (UserDir d)      { return sm_get_user_dir (d); }
string get (DocumentsDir d) { return sm_get_documents_dir (d); }

template<class D> void
print_dir (const char *s, D dir)
{
  printf ("%30s = '%s'\n", s, get (dir).c_str());
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  if (argc == 2)
    {
      printf ("### with pkg_data_dir='%s'\n\n", argv[1]);
      sm_set_pkg_data_dir (argv[1]);
    }

  print_dir ("INSTALL_DIR_BIN", INSTALL_DIR_BIN);
  print_dir ("INSTALL_DIR_TEMPLATES", INSTALL_DIR_TEMPLATES);
  print_dir ("INSTALL_DIR_INSTRUMENTS", INSTALL_DIR_INSTRUMENTS);
  print_dir ("INSTALL_DIR_FONTS", INSTALL_DIR_FONTS);
  printf ("\n");

  print_dir ("USER_DIR_INSTRUMENTS", USER_DIR_INSTRUMENTS);
  print_dir ("USER_DIR_CACHE", USER_DIR_CACHE);
  print_dir ("USER_DIR_DATA", USER_DIR_DATA);
  printf ("\n");

  print_dir ("DOCUMENTS_DIR_INSTRUMENS", DOCUMENTS_DIR_INSTRUMENTS);
  printf ("\n");

  printf ("%30s = '%s'\n", "sm_get_default_plan()", sm_get_default_plan().c_str());
  printf ("%30s = '%s'\n", "sm_get_cache_dir()", sm_get_cache_dir().c_str());
}
