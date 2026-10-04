import util as utl

class Field(object):
    def __init__(self, name, width = 0, val = None, left_pad = 0, right_pad = 0):
        self.offset = 0
        self.width = width

        self.__name = name
        self.__left_pad = left_pad
        self.__right_pad = right_pad
        self.__val = val

    def name(self):
        return self.__name

    def val(self):
        return self.__val

    def setval(self, val):
        self.__val = val

    def setwidth(self, width):
        self.width = width

    def val2hex(self):
        if self.__val != None:
            return self.__val
        return utl.str2hex('0x0') * self.width

    def hex2val(self, raw):
        return raw

    def populate_image(self):
        return utl.str2hex('0x0') * self.__left_pad + \
                self.val2hex() + \
                utl.str2hex('0x0') * self.__right_pad

    def extract_val(self, raw):
        _hex = raw
        if self.__left_pad != 0:
            _hex = _hex[self.__left_pad:]
        if self.__right_pad != 0:
            _hex = _hex[:-self.__right_pad]
        self.__val = self.hex2val(_hex)
        return self.__val

    def __len__(self):
        return self.__left_pad + self.width + self.__right_pad

    def replace(self, image, new):
        return utl.replace_bytes(
                old_bytes=image,
                pos=self.offset,
                new_bytes=new
                )

class FieldLe(Field):
    def __init__(self, name, width=4, val=0):
        super(FieldLe, self).__init__(name, width=width, val=val)

    def val2hex(self):
        return utl.str2le(str(self.__val), group_size=self.width) 

    def hex2val(self, raw):
        return utl.le2int(raw, self.width)

class FieldRes(Field):
    def __init__(self, name, width):
        super(FieldRes, self).__init__(name, width=width, val=None)

    def hex2val(self, raw):
        return raw

class FieldBinary(Field):
    def __init__(self, name, val, left_pad, right_pad):
        super(FieldBinary, self).__init__(name, width=len(val), val=val, left_pad=left_pad, right_pad=right_pad)

    def setval(self, val):
        super(FieldBinary, self).setval(val)
        super(FieldBinary, self).setwidth(len(val))
