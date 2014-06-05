import scalar
import container
from itertools import ifilter

def validate(descriptor):
    if any(type._UNLIMITED for _, type in descriptor[:-1]):
        raise Exception("unlimited field is not the last one")

def add_attributes(cls, descriptor):
    cls._SIZE = sum(type._SIZE for _, type in descriptor)
    cls._SIZE += sum(x._optional_type._SIZE for x in filter(lambda x: x._OPTIONAL, (type for _, type in descriptor)))
    cls._DYNAMIC = any(type._DYNAMIC for _, type in descriptor)
    cls._UNLIMITED = any(type._UNLIMITED for _, type in descriptor)
    cls._OPTIONAL = False
    cls._ALIGNMENT = max(type._ALIGNMENT for _, type in descriptor) if descriptor else 1
    cls._BOUND = None

def add_padding(cls, descriptor):

    class padder(object):
        def __init__(self):
            self.offset = 0
        def __call__(self, size, alignment):
            self.offset += size
            padding = (alignment - self.offset % alignment) % alignment
            self.offset += padding
            return padding

    sizes = [tp._SIZE for _, tp in descriptor]
    alignments = [tp._ALIGNMENT for _, tp in descriptor[1:]]
    if descriptor:
        alignments.append(cls._ALIGNMENT)
    paddings = map(padder(), sizes, alignments)
    descriptor[:] = [field + (padding,) for field, padding in zip(descriptor, paddings)]
    cls._SIZE += sum(paddings)

def add_null_padding(descriptor):
    descriptor[:] = [field + (0,) for field in descriptor]

def add_properties(cls, descriptor):
    for name, type, _ in descriptor:
        if issubclass(type, container.base_array):
            add_repeated(cls, name, type)
        elif issubclass(type, (struct, union)):
            add_composite(cls, name, type)
        else:
            add_scalar(cls, name, type)

def add_repeated(cls, field_name, field_type):
    def getter(self):
        field_value = self._fields.get(field_name)
        if field_value is None:
            field_value = field_type()
            field_value = self._fields.setdefault(field_name, field_value)
        return field_value
    def setter(self, new_value):
        raise Exception("assignment to array field not allowed")
    setattr(cls, field_name, property(getter, setter))
    if field_type._BOUND:
        substitute_len_field(cls, cls._descriptor, field_name, field_type)

def add_scalar(cls, field_name, field_type):
    def getter(self):
        if field_type._OPTIONAL and field_name not in self._fields:
            return None
        return self._fields.get(field_name, field_type._DEFAULT)
    def setter(self, new_value):
        if field_type._OPTIONAL and new_value is None:
            self._fields.pop(field_name, None)
        else:
            self._fields[field_name] = field_type._check(new_value)
    setattr(cls, field_name, property(getter, setter))
    if field_type._BOUND:
        substitute_len_field(cls, cls._descriptor, field_name, field_type)

def add_composite(cls, field_name, field_type):
    def getter(self):
        if field_type._OPTIONAL and field_name not in self._fields:
            return None
        if field_name not in self._fields:
            value = field_type()
            self._fields[field_name] = value
            return value
        else:
            return self._fields.get(field_name)
    def setter(self, new_value):
        if field_type._OPTIONAL and new_value is True:
            self._fields[field_name] = field_type()
        elif field_type._OPTIONAL and new_value is None:
            self._fields.pop(field_name, None)
        else:
            raise Exception("assignment to composite field not allowed")
    setattr(cls, field_name, property(getter, setter))

def substitute_len_field(cls, descriptor, container_name, container_tp):
    index, field = ifilter(lambda x: x[1][0] is container_tp._BOUND, enumerate(descriptor)).next()
    name, tp, padding = field
    bound_shift = container_tp._BOUND_SHIFT

    if tp._OPTIONAL:
        raise Exception("array must not be bound to optional field")
    if not issubclass(tp, (int, long)):
        raise Exception("array must be bound to an unsigned integer")

    class container_len(tp):
        _BOUND = container_name

        @staticmethod
        def _encode(value, endianness):
            return tp._encode(value + bound_shift, endianness)

        @staticmethod
        def _decode(data, endianness):
            value, size = tp._decode(data, endianness)
            array_guard = 65536
            if value > array_guard:
                raise Exception("decoded array length over %s" % array_guard)
            value -= bound_shift
            if value < 0:
                raise Exception("decoded array length smaller than shift")
            return value, size

    descriptor[index] = (name, container_len, padding)
    delattr(cls, name)

def get_padding_size(struct_obj, offset, alignment):
    if isinstance(struct_obj, struct_packed):
        return 0
    remainder = offset % alignment
    if not remainder:
        return 0
    return alignment - remainder

def get_padding(struct_obj, offset, alignment):
    return '\x00' * get_padding_size(struct_obj, offset, alignment)

def indent(lines, spaces):
    return "\n".join((spaces * " ") + i for i in lines.splitlines()) + "\n"

def field_to_string(name, type, value):
    if issubclass(type, container.base_array):
        return "".join(field_to_string(name, type._TYPE, elem) for elem in value)
    elif issubclass(type, (struct, union)):
        return "%s {\n%s}\n" % (name, indent(str(value), spaces = 2))
    elif issubclass(type, str):
        return "%s: %s\n" % (name, repr(value))
    elif issubclass(type, scalar.enum):
        return "%s: %s\n" % (name, type._int_to_name[value])
    else:
        return "%s: %s\n" % (name, value)

def encode_field(type, value, endianess):
    if issubclass(type, container.base_array):
        return value.encode(endianess)
    elif issubclass(type, (struct, union)):
        return value.encode(endianess, terminal = False)
    else:
        return type._encode(value, endianess)

def decode_field(parent, name, type, data, endianess, len_hints):
    if issubclass(type, container.base_array):
        return getattr(parent, name).decode(data, endianess, len_hints.get(name))
    elif issubclass(type, (struct, union)):
        return getattr(parent, name).decode(data, endianess, terminal = False)
    elif issubclass(type, str):
        value, size = type._decode(data, endianess, len_hints.get(name))
        setattr(parent, name, value)
        return size
    else:
        value, size = type._decode(data, endianess)
        if type._BOUND:
            len_hints[type._BOUND] = value
        else:
            setattr(parent, name, value)
        return size

def validate_copy_from(lhs, rhs):
    if not isinstance(rhs, lhs.__class__):
        raise TypeError("Parameter to copy_from must be instance of same class.")

def set_field(parent, name, rhs):
    lhs = getattr(parent, name)
    if isinstance(rhs, container.base_array):
        if issubclass(rhs._TYPE, (struct, union)):
            if rhs._DYNAMIC:
                del lhs[:]
                lhs.extend(rhs[:])
            else:
                for lhs_elem, rhs_elem in zip(lhs, rhs):
                    lhs_elem.copy_from(rhs_elem)
        else:
            lhs[:] = rhs[:]
    elif isinstance(rhs, (struct, union)):
        lhs.copy_from(rhs)
    else:
        parent._fields[name] = rhs

class struct(object):
    __slots__ = []

    def __init__(self):
        self._fields = {}

    def __str__(self):
        out = ""
        for name, type, _ in self._descriptor:
            value = getattr(self, name, None)
            if value is not None:
                out += field_to_string(name, type, value)
        return out

    def encode(self, endianess, terminal = True):
        data = ""
        # FIXME(kkryspin): Raw loop
        for name, type, padding in self._descriptor:

            data += get_padding(self, len(data), type._ALIGNMENT)

            value = getattr(self, name, None)
            if type._OPTIONAL and value is None:
                data += type._optional_type._encode(False, endianess)
                data += "\x00" * type._SIZE
            elif type._OPTIONAL:
                data += type._optional_type._encode(True, endianess)
                data += encode_field(type, value, endianess)
            elif type._BOUND and issubclass(type, (int, long)):
                array_value = getattr(self, type._BOUND)
                data += type._encode(len(array_value), endianess)
            else:
                data += encode_field(type, value, endianess)

        if self._descriptor and terminal and issubclass(type, (container.base_array, str)):
            pass
        else:
            data += get_padding(self, len(data), self._ALIGNMENT)

        return data

    def decode(self, data, endianess, terminal = True):
        len_hints = {}
        bytes_read = 0
        # FIXME(kkryspin): Raw loop
        for name, type, padding in self._descriptor:
            if type._OPTIONAL:
                value, size = type._optional_type._decode(data, endianess)
                data = data[size:]
                bytes_read += size
                if value:
                    setattr(self, name, True)
                else:
                    setattr(self, name, None)
                    data = data[type._SIZE:]
                    bytes_read += type._SIZE
                    continue

            padding = get_padding_size(self, bytes_read, type._ALIGNMENT)
            data = data[padding:]
            bytes_read += padding

            size = decode_field(self, name, type, data, endianess, len_hints)
            data = data[size:]
            bytes_read += size

        if self._descriptor and terminal and issubclass(type, (container.base_array, str)):
            pass
        else:
            padding = get_padding_size(self, bytes_read, self._ALIGNMENT)
            data = data[padding:]
            bytes_read += padding

        if terminal and data:
            raise Exception("not all bytes read")

        return bytes_read

    def copy_from(self, other):
        validate_copy_from(self, other)
        if other is self:
            return

        self._fields.clear()
        for name, rhs in other._fields.iteritems():
            set_field(self, name, rhs)

class struct_packed(struct):
    __slots__ = []

class struct_generator(type):
    def __new__(cls, name, bases, attrs):
        attrs["__slots__"] = ["_fields"]
        return super(struct_generator, cls).__new__(cls, name, bases, attrs)
    def __init__(cls, name, bases, attrs):
        if not hasattr(cls, "_generated"):
            cls._generated = True
            descriptor = cls._descriptor
            validate(descriptor)
            add_attributes(cls, descriptor)
            add_null_padding(descriptor) if issubclass(cls, struct_packed) else add_padding(cls, descriptor)
            add_properties(cls, descriptor)
        super(struct_generator, cls).__init__(name, bases, attrs)

def validate_union(descriptor):
    if any(type._DYNAMIC for _, type, _ in descriptor):
        raise Exception("dynamic types not allowed in union")
    if any(type._BOUND for _, type, _ in descriptor):
        raise Exception("bound array/bytes not allowed in union")
    if any(issubclass(type, container.base_array) for _, type, _ in descriptor):
        raise Exception("static array not implemented in union")
    if any(type._OPTIONAL for _, type, _ in descriptor):
        raise Exception("union with optional field disallowed")

def add_union_attributes(cls, descriptor):
    cls._discriminator_type = scalar.u32
    cls._SIZE = cls._discriminator_type._SIZE + max(type._SIZE for _, type, _ in descriptor)
    cls._DYNAMIC = False
    cls._UNLIMITED = False
    cls._OPTIONAL = False
    cls._ALIGNMENT = max(type._ALIGNMENT for _, type, _ in descriptor)
    cls._BOUND = None

def add_union_properties(cls, descriptor):
    add_union_discriminator(cls)
    for name, type, disc in descriptor:
        if issubclass(type, (struct, union)):
            add_union_composite(cls, name, type, disc)
        else:
            add_union_scalar(cls, name, type, disc)

def add_union_discriminator(cls):
    def getter(self):
        return self._discriminator
    def setter(self, new_value):
        field = next(ifilter(lambda x: new_value in (x[0], x[2]), self._descriptor), None)
        if field:
            name, type, disc = field
            if disc != self._discriminator:
                self._discriminator = disc
                self._fields = {}
        else:
            raise Exception("unknown discriminator")
    setattr(cls, "discriminator", property(getter, setter))

def add_union_scalar(cls, name, type, disc):
    def getter(self):
        if self._discriminator is not disc:
            raise Exception("currently field %s is discriminated" % self._discriminator)
        return self._fields.get(name, type._DEFAULT)
    def setter(self, new_value):
        if self._discriminator is not disc:
            raise Exception("currently field %s is discriminated" % self._discriminator)
        new_value = type._check(new_value)
        self._fields[name] = new_value
    setattr(cls, name, property(getter, setter))

def add_union_composite(cls, name, type, disc):
    def getter(self):
        if self._discriminator is not disc:
            raise Exception("currently field %s is discriminated" % self._discriminator)
        value = self._fields.get(name)
        if value is None:
            value = type()
            value = self._fields.setdefault(name, value)
        return value
    def setter(self, new_value):
        raise Exception("assignment to composite field not allowed")
    setattr(cls, name, property(getter, setter))

def get_discriminated_field(cls, discriminator):
    field = next((x for x in cls._descriptor if x[2] == discriminator), None)
    return field[:2] if field else None

class union(object):
    __slots__ = []

    def __init__(self):
        self._fields = {}
        self._discriminator = self._descriptor[0][2]

    def __str__(self):
        name, tp = get_discriminated_field(self, self._discriminator)
        value = getattr(self, name)
        return field_to_string(name, tp, value)

    def encode(self, endianess, terminal = True):
        name, tp = get_discriminated_field(self, self._discriminator)
        value = getattr(self, name)
        data = self._discriminator_type._encode(self._discriminator, endianess) + encode_field(tp, value, endianess)
        return data.ljust(self._SIZE, '\x00')

    def decode(self, data, endianess, terminal = True):
        disc, bytes_read = self._discriminator_type._decode(data, endianess)
        field = get_discriminated_field(self, disc)
        if not field:
            raise Exception("unknown discriminator")
        name, type = field
        self._discriminator = disc
        bytes_read += decode_field(self, name, type, data[bytes_read:], endianess, {})
        if len(data) < self._SIZE:
            raise Exception("not enough bytes")
        if terminal and len(data) > self._SIZE:
            raise Exception("not all bytes read")
        return self._SIZE

    def copy_from(self, other):
        validate_copy_from(self, other)
        if other is self:
            return

        self._fields.clear()
        self._discriminator = other._discriminator
        name, type = get_discriminated_field(self, self._discriminator)
        rhs = getattr(other, name)
        if issubclass(type, (struct, union)):
            lhs = getattr(self, name)
            lhs.copy_from(rhs)
        else:
            setattr(self, name, rhs)

class union_generator(type):
    def __new__(cls, name, bases, attrs):
        attrs["__slots__"] = ["_fields", "_discriminator"]
        return super(union_generator, cls).__new__(cls, name, bases, attrs)
    def __init__(cls, name, bases, attrs):
        descriptor = cls._descriptor
        validate_union(descriptor)
        add_union_attributes(cls, descriptor)
        add_union_properties(cls, descriptor)
        super(union_generator, cls).__init__(name, bases, attrs)
