# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: _py_TO_AddMessageFlowCmd_t.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='_py_TO_AddMessageFlowCmd_t.proto',
  package='',
  serialized_pb=_b('\n _py_TO_AddMessageFlowCmd_t.proto\"x\n\x19TO_AddMessageFlowCmd_t_pb\x12\x13\n\x0bucCmdHeader\x18\x01 \x03(\r\x12\r\n\x05MsgID\x18\x02 \x02(\r\x12\x11\n\tPQueueIdx\x18\x03 \x02(\r\x12\x12\n\nChannelIdx\x18\x04 \x02(\r\x12\x10\n\x08MsgLimit\x18\x05 \x02(\r')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)




_TO_ADDMESSAGEFLOWCMD_T_PB = _descriptor.Descriptor(
  name='TO_AddMessageFlowCmd_t_pb',
  full_name='TO_AddMessageFlowCmd_t_pb',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='ucCmdHeader', full_name='TO_AddMessageFlowCmd_t_pb.ucCmdHeader', index=0,
      number=1, type=13, cpp_type=3, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='MsgID', full_name='TO_AddMessageFlowCmd_t_pb.MsgID', index=1,
      number=2, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='PQueueIdx', full_name='TO_AddMessageFlowCmd_t_pb.PQueueIdx', index=2,
      number=3, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='ChannelIdx', full_name='TO_AddMessageFlowCmd_t_pb.ChannelIdx', index=3,
      number=4, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='MsgLimit', full_name='TO_AddMessageFlowCmd_t_pb.MsgLimit', index=4,
      number=5, type=13, cpp_type=3, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=36,
  serialized_end=156,
)

DESCRIPTOR.message_types_by_name['TO_AddMessageFlowCmd_t_pb'] = _TO_ADDMESSAGEFLOWCMD_T_PB

TO_AddMessageFlowCmd_t_pb = _reflection.GeneratedProtocolMessageType('TO_AddMessageFlowCmd_t_pb', (_message.Message,), dict(
  DESCRIPTOR = _TO_ADDMESSAGEFLOWCMD_T_PB,
  __module__ = '_py_TO_AddMessageFlowCmd_t_pb2'
  # @@protoc_insertion_point(class_scope:TO_AddMessageFlowCmd_t_pb)
  ))
_sym_db.RegisterMessage(TO_AddMessageFlowCmd_t_pb)


# @@protoc_insertion_point(module_scope)