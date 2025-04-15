/* Networking DHCPv4 client */

/*
 * Copyright (c) 2025 3mdeb Sp z o.o.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(MAIN);

#include <zephyr/kernel.h>
#include <zephyr/linker/sections.h>
// #include <errno.h>
#include <stdio.h>

#include <zephyr/net/net_context.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>

#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>

#define DHCP_OPTION_NTP (42)
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

static uint8_t ntp_server[4];

static struct net_mgmt_event_callback mgmt_cb;

static struct net_dhcpv4_option_callback dhcp_cb;

/* Wi-Fi Configuration */
#define WIFI_SSID "rpi3-hotspot"
#define WIFI_PASSWORD "rpi3-pass"
#define WIFI_SECURITY_TYPE WIFI_SECURITY_TYPE_PSK

#define NET_EVENT_WIFI_MASK                                                    \
  (NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT |          \
   NET_EVENT_WIFI_AP_ENABLE_RESULT | NET_EVENT_WIFI_AP_DISABLE_RESULT |        \
   NET_EVENT_WIFI_AP_STA_CONNECTED | NET_EVENT_WIFI_AP_STA_DISCONNECTED)

static struct net_if *sta_iface;
static struct net_mgmt_event_callback cb;

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint32_t mgmt_event, struct net_if *iface) {
  switch (mgmt_event) {
  case NET_EVENT_WIFI_CONNECT_RESULT: {
    LOG_INF("Connected to %s", WIFI_SSID);
    break;
  }
  case NET_EVENT_WIFI_DISCONNECT_RESULT: {
    LOG_INF("Disconnected from %s", WIFI_SSID);
    break;
  }
  case NET_EVENT_WIFI_AP_ENABLE_RESULT: {
    LOG_INF("AP Mode is enabled. Waiting for station to connect");
    break;
  }
  case NET_EVENT_WIFI_AP_DISABLE_RESULT: {
    LOG_INF("AP Mode is disabled.");
    break;
  }
  case NET_EVENT_WIFI_AP_STA_CONNECTED: {
    struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;

    LOG_INF("station: " MACSTR " joined ", sta_info->mac[0], sta_info->mac[1],
            sta_info->mac[2], sta_info->mac[3], sta_info->mac[4],
            sta_info->mac[5]);
    break;
  }
  case NET_EVENT_WIFI_AP_STA_DISCONNECTED: {
    struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;

    LOG_INF("station: " MACSTR " leave ", sta_info->mac[0], sta_info->mac[1],
            sta_info->mac[2], sta_info->mac[3], sta_info->mac[4],
            sta_info->mac[5]);
    break;
  }
  default:
    break;
  }
}

static void start_dhcpv4_client(struct net_if *iface, void *user_data) {
  ARG_UNUSED(user_data);

  LOG_INF("Start on %s: index=%d", net_if_get_device(iface)->name,
          net_if_get_by_iface(iface));
  net_dhcpv4_start(iface);
}

static void handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
                    struct net_if *iface) {
  int i = 0;

  if (mgmt_event != NET_EVENT_IPV4_ADDR_ADD) {
    return;
  }

  for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
    char buf[NET_IPV4_ADDR_LEN];

    if (iface->config.ip.ipv4->unicast[i].ipv4.addr_type != NET_ADDR_DHCP) {
      continue;
    }

    LOG_INF(
        "   Address[%d]: %s", net_if_get_by_iface(iface),
        net_addr_ntop(AF_INET,
                      &iface->config.ip.ipv4->unicast[i].ipv4.address.in_addr,
                      buf, sizeof(buf)));
    LOG_INF("    Subnet[%d]: %s", net_if_get_by_iface(iface),
            net_addr_ntop(AF_INET, &iface->config.ip.ipv4->unicast[i].netmask,
                          buf, sizeof(buf)));
    LOG_INF(
        "    Router[%d]: %s", net_if_get_by_iface(iface),
        net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, buf, sizeof(buf)));
    LOG_INF("Lease time[%d]: %u seconds", net_if_get_by_iface(iface),
            iface->config.dhcpv4.lease_time);
  }
}

static void option_handler(struct net_dhcpv4_option_callback *cb, size_t length,
                           enum net_dhcpv4_msg_type msg_type,
                           struct net_if *iface) {
  char buf[NET_IPV4_ADDR_LEN];

  LOG_INF("DHCP Option %d: %s", cb->option,
          net_addr_ntop(AF_INET, cb->data, buf, sizeof(buf)));
}

static int connect_to_wifi(void) {
  struct wifi_connect_req_params wifi_params = {
      .ssid = WIFI_SSID,
      .ssid_length = strlen(WIFI_SSID),
      .psk = WIFI_PASSWORD,
      .psk_length = strlen(WIFI_PASSWORD),
      .security = WIFI_SECURITY_TYPE,
  };
  LOG_INF("Connecting to Wi-Fi network %s...", WIFI_SSID);

  sta_iface = net_if_get_wifi_sta();
  int ret;
  ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, sta_iface, &wifi_params,
                 sizeof(wifi_params));
  if (ret != 0) {
    LOG_ERR("Failed to connect to Wi-Fi network: %d", ret);
    return ret;
  }

  LOG_INF("Connected to Wi-Fi network %s", WIFI_SSID);

  return 0;
}

int main(void) {
  net_mgmt_init_event_callback(&cb, wifi_event_handler, NET_EVENT_WIFI_MASK);
  net_mgmt_add_event_callback(&cb);

  net_mgmt_init_event_callback(&mgmt_cb, handler, NET_EVENT_IPV4_ADDR_ADD);
  net_mgmt_add_event_callback(&mgmt_cb);
  net_dhcpv4_init_option_callback(&dhcp_cb, option_handler, DHCP_OPTION_NTP,
                                  ntp_server, sizeof(ntp_server));
  net_dhcpv4_add_option_callback(&dhcp_cb);

  connect_to_wifi();

  LOG_INF("Run dhcpv4 client");
  net_if_foreach(start_dhcpv4_client, NULL);

  return 0;
}
