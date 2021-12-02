/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/dinputproxy
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <set>
#include <string>

struct proxyconfig;

extern struct proxyconfig configx;

struct proxyconfig
{
  char *dllpath;
  std::set<unsigned int> allowclass;
  std::set<std::string> blacklist;
  bool mask_xone_ctrl;
  bool find_all_dlls;
};


void read_conf(void);

#endif