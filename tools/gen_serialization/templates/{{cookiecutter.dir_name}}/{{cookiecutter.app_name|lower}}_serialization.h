/* This file was autogenerated from template version 0.0.0.1 and autogenerator version {{cookiecutter.autogen_version}} */

#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_lib.h"
{% for proto_msg, proto_data in cookiecutter.proto_msgs.iteritems() -%}
#include "{{proto_msg[:-3]}}.pb.h"
{% endfor %}
#ifdef __cplusplus
extern "C" {
#endif

{% for proto_msg, proto_data in cookiecutter.proto_msgs.iteritems() -%}
uint32 {{proto_data.airliner_msg}}_Enc(const {{proto_data.airliner_msg}} *inObject, char *inOutBuffer, uint32 inSize);
uint32 {{proto_data.airliner_msg}}_Dec(const char *inBuffer, uint32 inSize, {{proto_data.airliner_msg}} *inOutObject);

{% endfor -%}

#ifdef __cplusplus
} /* extern "C" */
#endif
