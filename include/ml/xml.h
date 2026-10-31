#ifndef ML_XML_H
#define ML_XML_H

int ml_xml_status_ok(const char *xml);
int ml_xml_get_int(const char *xml, const char *tag, int *out);
int ml_xml_get_str(const char *xml, const char *tag, char *out, int out_sz);
int ml_xml_get_hex(const char *xml, const char *tag, unsigned char *out, int out_sz, int *out_len);

#endif
