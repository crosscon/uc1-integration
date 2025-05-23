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

/* socket */
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

/* SSL */
#ifndef WOLFSSL_USER_SETTINGS
    #include <wolfssl/options.h>
#endif
#include <wolfssl/ssl.h>
#include "ssl_certs.h"
#define HEAP_HINT  NULL

#define DHCP_OPTION_NTP (42)
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

static uint8_t ntp_server[4];

static struct net_mgmt_event_callback mgmt_cb;

static struct net_dhcpv4_option_callback dhcp_cb;

/*********** Socket start ***********/

#define SERVER_ADDR  "192.168.66.173"
#define SERVER_PORT  11111

/* DO NOT use this in production. You should implement a way
 * to get the current date. */
static int verifyIgnoreDateError(int preverify, WOLFSSL_X509_STORE_CTX* store)
{
    if (store->error == ASN_BEFORE_DATE_E)
        return 1; /* override error */
    else
        return preverify;
}

static int test_socket_connection(void) {
  struct sockaddr_in servAddr; 
  int                ret;
  int                sock;
  char               buff[256];
  size_t             len;
  const char msg[] = "Hello from Zephyr!\n";
  char               wolfsslErrorStr[80];

  /* declare wolfSSL objects */
  WOLFSSL_CTX* ctx;
  WOLFSSL*     ssl;
  WOLFSSL_CIPHER* cipher;

  LOG_INF("Creating socket...");

  /* Create a socket that uses an internet IPv4 address,
   * Sets the socket to be stream based (TCP) */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0) {
		LOG_ERR("Failed to create HTTP socket (%d)", -errno);
	}

  LOG_INF("Socket created successfuly");

  /* Initialize the server address struct with zeros */ 
	memset(&servAddr, 0, sizeof(servAddr));
  LOG_INF("memset");

  /* Fill in the server address */
  servAddr.sin_family = AF_INET;             /* using IPv4      */
  servAddr.sin_port   = htons(SERVER_PORT);  /* on SERVER_PORT  */
  LOG_INF("server settings");

	if (inet_pton(AF_INET, SERVER_ADDR, &servAddr.sin_addr) != 1) {
	  LOG_ERR("Invalid server address");
	  ret = -1; 
	}

  /* Connect to the server */
  LOG_INF("Connecting to the server...");
  if ((ret = connect(sock, (struct sockaddr*) &servAddr, sizeof(servAddr)))
       == -1) {
      LOG_ERR("Failed to connect");
      goto end;
  }

  /*---------------------------------*/
  /* Start of wolfSSL initialization and configuration */
  /*---------------------------------*/
  /* Initialize wolfSSL */
  LOG_INF("wolfssl init");
  if ((ret = wolfSSL_Init()) != WOLFSSL_SUCCESS) {
      fprintf(stderr, "ERROR: Failed to initialize the library\n");
      goto socket_cleanup;
  }

#ifdef DEBUG_WOLFSSL
    wolfSSL_Debugging_ON();
#endif

  LOG_INF("wolfssl ctx");
  /* Create and initialize WOLFSSL_CTX */
#ifdef USE_TLSV13
  LOG_INF("Using TLS v1.3");
  ctx = wolfSSL_CTX_new_ex(wolfTLSv1_3_client_method_ex(HEAP_HINT), HEAP_HINT);
#else
  LOG_INF("Using TLS v1.2");
  ctx = wolfSSL_CTX_new_ex(wolfTLSv1_2_client_method_ex(HEAP_HINT), HEAP_HINT);
#endif
  if (ctx == NULL) {
    LOG_ERR("Failed to create WOLFSSL_CTX");
    ret = -1;
    goto socket_cleanup;
  }

  /* Load client certificate into WOLFSSL_CTX */
  LOG_INF("Loading client certificate...");
  if ((ret = wolfSSL_CTX_use_certificate_buffer(ctx, client_cert_der,
               client_cert_der_len, WOLFSSL_FILETYPE_ASN1)) != WOLFSSL_SUCCESS) {
    LOG_ERR("Failed to load client certificate.");
    goto ctx_cleanup;
  }

  /* Load client key into WOLFSSL_CTX */
  LOG_INF("Loading client key...");
  if ((ret = wolfSSL_CTX_use_PrivateKey_buffer(ctx, client_key_der,
               client_key_der_len, WOLFSSL_FILETYPE_ASN1)) != WOLFSSL_SUCCESS) {
    LOG_ERR("Failed to load client key.");
    goto ctx_cleanup;
  }

  /* Load CA certificate into WOLFSSL_CTX for validating peer */
  LOG_INF("Loading CA certificate...");
  if ((ret = wolfSSL_CTX_load_verify_buffer_ex(ctx, ca_cert_der,
          ca_cert_der_len, WOLFSSL_FILETYPE_ASN1, 0,
          /* DO NOT use this in production. You should
           * implement a way to get the current date. */
          WOLFSSL_LOAD_FLAG_DATE_ERR_OKAY)) !=
          WOLFSSL_SUCCESS) {
    LOG_ERR("Failed to load client key.");
    goto ctx_cleanup;
  }

  /* validate peer certificate */
  wolfSSL_CTX_set_verify(ctx,
    WOLFSSL_VERIFY_PEER|WOLFSSL_VERIFY_FAIL_IF_NO_PEER_CERT,
    verifyIgnoreDateError);

  /* Create a WOLFSSL object */
  LOG_INF("Creating SSL object...");
  if ((ssl = wolfSSL_new(ctx)) == NULL) {
      LOG_ERR("Failed to create WOLFSSL object");
      ret = -1;
      goto ctx_cleanup;
  }

  /* Attach wolfSSL to the socket */
  LOG_INF("Attaching SSL object...");
  if ((ret = wolfSSL_set_fd(ssl, sock)) != WOLFSSL_SUCCESS) {
      LOG_ERR("Failed to set the file descriptor");
      goto cleanup;
  }

  /* Connect to wolfSSL on the server side */
  LOG_INF("Connecting to server...");
  if ((ret = wolfSSL_connect(ssl)) != WOLFSSL_SUCCESS) {
      wolfSSL_ERR_error_string(wolfSSL_get_error(ssl, ret), wolfsslErrorStr);
      LOG_ERR("Failed to connect to server: %s", wolfsslErrorStr);
      goto cleanup;
  }

  cipher = wolfSSL_get_current_cipher(ssl);
  LOG_INF("SSL cipher suite is %s", wolfSSL_CIPHER_get_name(cipher));

  /* Construct a message for the server */
  LOG_INF("Constructing message for server...");
  memset(buff, 0, sizeof(buff));
  memcpy(buff, msg, sizeof(msg));
  len = strnlen(buff, sizeof(buff));

  /* Send the message to the server */
  LOG_INF("Sending message to server...");
  if ((ret = wolfSSL_write(ssl, buff, len)) != len) {
      LOG_ERR("Failed to write entire message");
      LOG_ERR("%d bytes of %d bytes were sent", ret, (int) len);
      goto cleanup;
  }

  /* Read the server data into our buff array */
  LOG_INF("Reading message from server...");
  memset(buff, 0, sizeof(buff));
  if ((ret = wolfSSL_read(ssl, buff, sizeof(buff)-1)) == -1) {
      LOG_ERR("Failed to read from server");
      goto cleanup;
  }

  /* Print to stdout any data the server sends */
  LOG_INF("Server: %s", buff);

  /* Bidirectional shutdown */
  LOG_INF("Starting SSL shutdown...");
  while (wolfSSL_shutdown(ssl) == WOLFSSL_SHUTDOWN_NOT_DONE) {
      LOG_INF("SSL shutdown not complete");
  }

  LOG_INF("SSL shutdown complete");

  ret = 0;

  /* Cleanup and return */
cleanup:
  wolfSSL_free(ssl);      /* Free the wolfSSL object                  */
ctx_cleanup:
  wolfSSL_CTX_free(ctx);  /* Free the wolfSSL context object          */
  wolfSSL_Cleanup();      /* Cleanup the wolfSSL environment          */
socket_cleanup:
  close(sock)    ;          /* Close the connection to the server       */
end:
  LOG_INF("ret: %d", ret);
  return ret;               /* Return reporting a success               */
}

/*********** Socket end ***********/

/* Wi-Fi Configuration */
#include "wifi_config.h"
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
  LOG_INF("Hello in WiFi App!");
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

  /* Wait for network to settle */
  /* TODO: find a better way of ensuring this */
  sleep(5);
  test_socket_connection();

  return 0;
}
