#pragma once

namespace Flow {

struct Definition {
  struct {
    bool process = false;
    bool src = false;
    bool dst = false;
  } ip;

  struct {
    bool process = false;
    bool src = false;
    bool dst = false;
  } ipv6;

  struct {
    bool process = false;
    bool src = false;
    bool dst = false;
  } tcp;

  struct {
    bool process = false;
    bool src = false;
    bool dst = false;
  } udp;

  struct {
    bool process = false;
    bool id = false;
  } dot1q;

  struct {
    bool process = false;
    bool label = false;
  } mpls;

  struct {
    bool process = false;
    bool vni = false;
  } vxlan;
};

} // namespace Flow
