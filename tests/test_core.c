#include "ml/common.h"
#include "ml/config.h"
#include "ml/gs.h"
#include "ml/rtp.h"
#include "ml/xml.h"

#include <stdio.h>

static int fails;

static void expect(int cond, const char *msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        fails++;
    }
}

static void test_xml(void) {
    const char *xml =
        "<?xml version=\"1.0\"?><root status_code=\"200\"><hostname>PC</hostname>"
        "<PairStatus>1</PairStatus><HttpsPort>47984</HttpsPort></root>";
    char host[32];
    int n = 0;
    expect(ml_xml_status_ok(xml), "status 200 attr");
    expect(ml_xml_get_str(xml, "hostname", host, sizeof(host)) == ML_OK, "hostname");
    expect(!strcmp(host, "PC"), "hostname value");
    expect(ml_xml_get_int(xml, "PairStatus", &n) == ML_OK && n == 1, "paired");
    expect(ml_xml_get_int(xml, "HttpsPort", &n) == ML_OK && n == 47984, "https port");
}

static void test_rtp(void) {
    unsigned char pkt[20] = {0x80, 96, 0x00, 0x01, 0, 0, 0, 0, 0, 0, 0, 1, 0x65, 1, 2, 3, 4, 5, 6, 7};
    MlRtpHeader hdr;
    unsigned char nal[64];
    int nal_len = 0, done = 0;
    expect(ml_rtp_parse(pkt, 20, &hdr) == ML_OK, "rtp parse");
    expect(hdr.payload_type == 96, "pt");
    expect(hdr.seq == 1, "seq");
    expect(hdr.header_len == 12, "hdr len");
    expect(ml_h264_depay(pkt + 12, 8, nal, sizeof(nal), &nal_len, &done) == ML_OK, "depay");
    expect(done == 1 && nal_len == 12, "single nal size");
    expect(nal[0] == 0 && nal[1] == 0 && nal[2] == 0 && nal[3] == 1, "start code");
    expect(nal[4] == 0x65, "idr type");
}

static void test_fu_a(void) {
    unsigned char start[] = {0x7C, 0x85, 0xAA, 0xBB};
    unsigned char mid[] = {0x7C, 0x05, 0xCC};
    unsigned char end[] = {0x7C, 0x45, 0xDD};
    unsigned char nal[64];
    int nal_len = 0, done = 0;
    expect(ml_h264_depay(start, 4, nal, sizeof(nal), &nal_len, &done) == ML_OK && done == 0, "fu start");
    expect(ml_h264_depay(mid, 3, nal, sizeof(nal), &nal_len, &done) == ML_OK && done == 0, "fu mid");
    expect(ml_h264_depay(end, 3, nal, sizeof(nal), &nal_len, &done) == ML_OK && done == 1, "fu end");
    expect(nal[4] == 0x65, "reconstructed nal hdr");
    expect(nal_len == 4 + 1 + 2 + 1 + 1, "fu total");
}

static void test_config(void) {
    MlConfig c;
    expect(ml_config_load_text(&c, "host=10.0.0.5\nwidth=1280\nbitrate=8000\n") == ML_OK, "ini");
    expect(!strcmp(c.host, "10.0.0.5"), "host");
    expect(c.width == 1280, "width");
    expect(c.bitrate_kbps == 8000, "br");
    expect(c.fps == 60, "fps default");
}

static void test_uuid(void) {
    char u[48];
    ml_gs_make_uuid(u, sizeof(u));
    expect(strlen(u) == 36, "uuid length");
    expect(u[8] == '-' && u[13] == '-' && u[18] == '-' && u[23] == '-', "uuid dashes");
}

int main(void) {
    test_xml();
    test_rtp();
    test_fu_a();
    test_config();
    test_uuid();
    if (fails) {
        fprintf(stderr, "%d failed\n", fails);
        return 1;
    }
    puts("ok");
    return 0;
}
