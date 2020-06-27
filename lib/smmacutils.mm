// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#import <Foundation/Foundation.h>

#include "smutils.hh"

#include <string>

namespace SpectMorph
{
  
std::string
sm_mac_documents_dir()
{
  NSArray *paths = NSSearchPathForDirectoriesInDomains (NSDocumentDirectory, NSUserDomainMask, YES);
  NSString *documentsDirectory = [paths objectAtIndex:0];
  return [documentsDirectory UTF8String];
}

std::string
sm_mac_application_support_dir()
{
  NSArray *paths = NSSearchPathForDirectoriesInDomains (NSApplicationSupportDirectory, NSUserDomainMask, YES);
  NSString *applicationSupportDirectory = [paths objectAtIndex:0];
  return [applicationSupportDirectory UTF8String];
}

}
