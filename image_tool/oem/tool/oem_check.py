from binascii import hexlify
import os
import sys
from Crypto.Protocol import KDF
import common.logger as log
import common.util as utl

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/../..' % SCRIPT_DIR)
sys.path.append(WORK_DIR)

import common.util as util
import common.logger as log
from common.area_tool import AreaTool, PublicKey

# The values in OTP and the values in the mirror may conflict, and they are verified according to the following criteria.
# The security algorithm is based on the configuration in OTP. If the value in the mirror is inconsistent with the configuration in OTP, an error will be reported;
# For the signature verification enablement:
#   - It is determined by the values in the mirror and OTP together;
#   - If OTP is enabled but not in the mirror, an error will be reported;
#   - If OTP is not enabled but enabled in the mirror, verification will be performed;
#   - If neither OTP nor the mirror is enabled, the verification will be skipped;
# The encryption enablement is determined by the values in the mirror and OTP together, and the check policy same as the verification enable configurations.


class OneTimeProgrammable:
    def __init__(self, file=None):
        self.__soc_tee_enable = None
        self.__ree_verify_enable = None
        self.__ree_hash_flash_root_key = None
        self.__msid = None
        self.__params_ver = None
        self.__ree_boot_ver = None
        self.__tee_verify_enable = None
        self.__tee_hash_flash_root_key = None
        self.__tee_boot_ver = None
        self.__tee_os_ver = None
        self.__scs_alg_sel = None
        self.__ree_dec_enable = None
        self.__obfu_mrk1 = None
        self.__tee_dec_enable = None
        self.load(file)

    def load(self, file):
        import json
        with open(file, 'r') as f:
            obj = json.load(f)
            self.__soc_tee_enable = int(obj['soc_tee_enable'], 16)
            self.__ree_verify_enable = int(obj['ree_verify_enable'], 16)
            self.__ree_hash_flash_root_key = \
                util.str2hex(obj["ree_hash_flash_root_key"])
            self.__msid = int(obj['msid'], 16)
            self.__params_ver = int(obj['params_ver'], 16)
            self.__ree_boot_ver = int(obj['ree_boot_ver'], 16)

            self.__tee_verify_enable = int(obj['tee_verify_enable'], 16)
            self.__tee_hash_flash_root_key = \
                util.str2hex(obj["tee_hash_flash_root_key"])
            self.__tee_boot_ver = int(obj['tee_boot_ver'], 16)
            self.__tee_os_ver = int(obj['tee_os_ver'], 16)

            self.__scs_alg_sel = int(obj['scs_alg_sel'], 16)
            self.__ree_dec_enable = int(obj['ree_dec_enable'], 16)
            self.__tee_dec_enable = int(obj['tee_dec_enable'], 16)
            self.__obfu_mrk1 = util.str2hex(obj['obfu_mrk1'])

    def ree_verify_enable(self):
        return self.__ree_verify_enable != 0x42

    def tee_verify_enable(self):
        return self.__tee_verify_enable != 0x42

    def soc_tee_enable(self):
        return self.__soc_tee_enable != 0x42

    def soc_tee_enable_val(self):
        return self.__soc_tee_enable

    def tee_dec_enable(self):
        return self.__tee_dec_enable != 0x0

    def ree_dec_enable(self):
        return self.__ree_dec_enable != 0x0

    def params_ver(self, mask):
        return bin(self.__params_ver & mask).count('1')

    def ree_boot_ver(self, mask):
        return bin(self.__ree_boot_ver & mask).count('1')

    def tee_boot_ver(self, mask):
        return bin(self.__tee_boot_ver & mask).count('1')

    def tee_os_ver(self, mask):
        return bin(self.__tee_os_ver & mask).count('1')

    def msid(self, mask):
        return self.__msid & mask

    def ree_hash_flash_root_key(self):
        return self.__ree_hash_flash_root_key

    def tee_hash_flash_root_key(self):
        return self.__tee_hash_flash_root_key

    def obfu_mrk1(self):
        return self.__obfu_mrk1

    def scs_alg_sel(self):
        return "RSA3072+SHA256+AES" if self.__scs_alg_sel == 0 \
                else "SM2+SM3+SM4"


class Field(object):
    def __init__(self, name, width=0, val=None, left_pad=0, right_pad=0):
        self.__offset = 0
        self.__width = width

        self.__name = name
        self.__left_pad = left_pad
        self.__right_pad = right_pad
        self.__val = val

    def name(self):
        return self.__name

    def set_offset(self, offset):
        self.__offset = offset

    def offset(self):
        return self.__offset

    def val(self):
        return self.__val

    def set_val(self, val):
        self.__val = val

    def dump(self):
        return utl.hex2str(self.val())

    def width(self):
        return self.__width

    def val2hex(self):
        if self.val() is not None:
            return self.val()
        return utl.str2hex('0x0') * self.width()

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
        self.set_val(self.hex2val(_hex))
        return self.val()

    def __len__(self):
        return self.__left_pad + self.width() + self.__right_pad

    def replace(self, image, new):
        return utl.replace_bytes(
                old_bytes=image,
                pos=self.offset(),
                new_bytes=new
                )


class ImageArea(object):
    def __init__(self, name, field_list=[]):
        self.__name = name
        self.__fields = field_list
        self.__area_info = {}

    def parse(self, raw):
        offset = 0
        area_info = {}
        for field in self.__fields:
            field.set_offset(offset)
            field.__val = field.extract_val(raw[offset: offset + len(field)])
            area_info[field.name()] = field
            offset += len(field)
        area_info["length"] = offset
        self.__area_info = area_info
        self.__raw = raw
        return area_info

    def raw(self):
        return self.__raw

    def area_info(self):
        return self.__area_info

    def set_area_info(self, area_info):
        self.__area_info = area_info

    def check(self, static_info):
        for field_name, val in static_info.items():
            if val is not None and self.__area_info[field_name].val() != val:
                log.error("{}: {} check fail: {:#x} expect {:#x}"
                          .format(self.name(), field_name,
                                  self.__area_info[field_name].val(), val))
                return False
            log.info("{}: {} check pass. ".format(self.name(), field_name))
        return True

    def build(self):
        area = utl.combine_bytes(*list(
                self.__fields.map(
                    lambda field: field.populate_image()
                    )))
        return area

    def fields(self):
        return self.__fields

    def name(self):
        return self.__name

    def dump(self):
        log.info("========== {} ==========".format(self.__name))
        for k, v in self.area_info().items():
            if k == "length":
                log.info("area length: {:#x}".format(v))
            elif type(v.val) is int:
                log.info("{}, {:#x}".format(k, v.val))
            else:
                log.info("key:{}, val:{}".format(k, v.dump()))
        log.info("=========== end =========")
        log.info("")


class FieldLe(Field):
    def __init__(self, name, width=4, val=0):
        super(FieldLe, self).__init__(name, width=width, val=val)

    def val2hex(self):
        return util.str2le(str(self.val()), group_size=self.width())

    def hex2val(self, raw):
        return util.le2int(raw, self.width())

    def dump(self):
        return '0x{:x}'.format(self.val())


class FieldRes(Field):
    def __init__(self, name, width):
        super(FieldRes, self).__init__(name, width=width, val=None)

    def hex2val(self, raw):
        return raw


class RootPublicKeysArea(ImageArea):
    def __init__(self, key_name):
        structure_length = 0x200
        key_length = 0x184
        fields = [
                FieldLe("image_id"),
                FieldLe("structure_version"),
                FieldLe("structure_length"),
                FieldLe("key_owner_id"),
                FieldLe("key_id"),
                FieldLe("key_alg"),
                FieldLe("ecc_curve_type"),
                FieldLe("key_length"),
                FieldLe("verify_enable_ext"),
                FieldRes("Reserved", structure_length - 36 - key_length),
                Field("flash_root_public_key", key_length),
                ]

        self.__key_name = key_name
        super(RootPublicKeysArea, self).__init__(key_name + "_rootkey", fields)

    def check(self, varity_enable, hash_alg, hash_val):
        structure_length = 0x200
        key_length = 0x184
        static_info = {
                "image_id": 0x4BA5C31E if self.__key_name == "ree" else 0x4B96B41E if self.__key_name == "tee" else 0x4B2D4B1E,
                "structure_version": 0x00010000,
                "structure_length": structure_length,
                "key_owner_id": 0x0,
                "key_id": 0x0,
                "key_alg": 0x2A13C834,
                "ecc_curve_type": None,
                "key_length": key_length,
                # "verify_enable_ext": 0x2A13C812 if  otp.ree_verify_enable() else None,
        }

        if not super(RootPublicKeysArea, self).check(static_info):
            return False

        if not varity_enable and not self.area_info()["verify_enable_ext"] == 0x2A13C812:
            return True

        field = self.area_info()["verify_enable_ext"]
        length = self.area_info()["length"]
        raw = field.replace(self.raw(), utl.str2hex('00' * field.width()))
        digest = hash_alg(raw[:length])
        if digest != hash_val:
            log.error("{}: {} check fail: {} expect {}".format(self.name(), "hash_val", utl.hex2str(digest), utl.hex2str(hash_val)))
            return False
        return True


class GSLKeyArea(ImageArea):
    def __init__(self):
        structure_length = 0x400
        key_length = 0x184
        signature_length = 0x180
        fields = [
                FieldLe("image_id"),
                FieldLe("structure_version"),
                FieldLe("structure_length"),
                FieldLe("signature_length"),
                FieldLe("key_owner_id"),
                FieldLe("key_id"),
                FieldLe("key_alg"),
                FieldLe("ecc_curve_type"),
                FieldLe("key_length"),
                FieldLe("gsl_key_version_ext"),
                FieldLe("mask_gsl_key_version_ext"),
                FieldLe("msid_ext"),
                FieldLe("mask_msid_ext"),
                FieldLe("maintenance_mode"),
                Field("die_id", width=16),
                FieldRes("reserved", structure_length - 72 - key_length - signature_length),
                Field("gsl_external_public_key", key_length),
                Field("sig_tee_boot_key_area", signature_length),
                ]

        super(GSLKeyArea, self).__init__("gsl_key_area", fields)

    def check(self, verify):
        static_info = {
                "image_id": 0x4BB4D21E,
                "structure_version": 0x00010000,
                "structure_length": 0x400,
                "signature_length": 0x180,
                "key_id": 0x0,
                "key_alg": 0x2A13C834,
                "key_length": 0x184,
                }

        if not super(GSLKeyArea, self).check(static_info):
            return False

        field = self.area_info()["sig_tee_boot_key_area"]
        length = len(field)

        if not verify(self.raw()[:field.offset()], self.raw()[field.offset():field.offset() + length]):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

class GSLCodeInfo(ImageArea):
    def __init__(self):
        fmc_cfg_data_len = 0x12 # dummy
        structure_length = 0x400
        signature_length = 0x180
        fields = [
                FieldLe("image_id"),
                FieldLe("structure_version"),
                FieldLe("structure_length"),
                FieldLe("signature_length"),
                FieldLe("gsl_code_version_ext"),
                FieldLe("mask_gsl_code_version_ext"),
                FieldLe("msid"),
                FieldLe("mask_msid"),
                FieldLe("gsl_code_area_addr"),
                FieldLe("gsl_code_area_len"),
                Field("gsl_code_area_hash", 32),
                FieldLe("gsl_enc_flag"),
                Field("key_material_gsl", 16),
                FieldRes("reserved1", 16),
                Field("iv", 16),
                FieldRes("reserved2", 8),
                FieldLe("text_segment_size"),
                FieldRes("reserved3", 4),
                Field("fmc_cfg_data", fmc_cfg_data_len),
                FieldRes("reserved2", structure_length - 140 - fmc_cfg_data_len - signature_length),
                Field("sig_gsl_info", signature_length),
                ]
        super(GSLCodeInfo, self).__init__("gsl_info_area", fields)

    def check(self, dec_enable, verify):
        static_info = {
                "image_id": 0x4BB4D22D,
                "structure_version": 0x00010000,
                "structure_length": 0x400,
                "signature_length": 0x180,
                "gsl_code_area_addr": 0x0,
                }

        if not super(GSLCodeInfo, self).check(static_info):
            return False

        if dec_enable:
            if self.area_info()["gsl_enc_flag"].val() == 0x3c7896e1:
                log.error("{}: Enc_Flag check fail: dec_enable otp has burned, but it is a plaintext image.".format(self.name()))
                return False

        field = self.area_info()["sig_gsl_info"]
        length = len(field)
        raw = self.raw()

        if not verify(raw[:field.offset()], raw[field.offset():field.offset() + length]):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        return True


class REEBootKeyArea(ImageArea):
    def __init__(self):
        structure_length = 0x400
        key_length = 0x184
        signature_length = 0x180
        fields = [
                FieldLe("image_id"),
                FieldLe("structure_version"),
                FieldLe("structure_length"),
                FieldLe("signature_length"),
                FieldLe("key_owner_id"),
                FieldLe("key_id"),
                FieldLe("key_alg"),
                FieldLe("ecc_curve_type"),
                FieldLe("key_length"),
                FieldLe("ree_key_version_ext"),
                FieldLe("mask_ree_key_version_ext"),
                FieldLe("msid_ext"),
                FieldLe("mask_msid_ext"),
                FieldLe("maintenance_mode"),
                Field("die_id", 16),
                FieldRes("reserved", structure_length - 72 - key_length - signature_length),
                Field("ree_boot_external_public_key", key_length),
                Field("sig_ree_key_area", signature_length)
                ]

        super(REEBootKeyArea, self).__init__("ree_boot_key_area", fields)

    def check(self, verify):
        static_info = {
                "image_id": 0x4B1E3C1E,
                "structure_version": 0x00010000,
                "structure_length": 0x400,
                "signature_length": 0x180,
                "key_owner_id": 0x0,
                "key_id": 0x0,
                "key_alg": 0x2A13C834,
                "key_length": 0x184,
                }

        if not super(REEBootKeyArea, self).check(static_info):
            return False

        field = self.area_info()["sig_ree_key_area"]
        length = len(field)
        if not verify(self.raw()[:field.offset()],
                      self.raw()[field.offset():field.offset() + length]):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True


class ParamsAreaInfo(ImageArea):
    def __init__(self):
        signature_length = 0x180
        fields = [
                FieldLe("image_id"),
                FieldLe("structure_version"),
                FieldLe("structure_length"),
                FieldLe("signature_length"),
                FieldLe("params_version_ext"),
                FieldLe("mask_params_version_ext"),
                FieldLe("msid_ext"),
                FieldLe("mask_msid_ext"),
                FieldLe("params_area_addr"),
                FieldLe("params_area_len"),
                FieldLe("params_area_num")
                ] \
                + [Field("params_area_hash" + str(i), 32) for i in range(8)] \
                + [FieldLe("board_index_hash_table" + str(i), 1) for i in range(8)] \
                + [FieldRes("reserved", 0x14c),
                   Field("sig_params_info", signature_length)]

        super(ParamsAreaInfo, self).__init__("param_info_area", fields)

    def check(self, verify):
        static_info = {
                "image_id" : 0x4B87A52D,
                "structure_version": 0x00010000,
                "structure_length": 0x400,
                "signature_length": 0x180,
                }

        if not super(ParamsAreaInfo, self).check(static_info):
            return False

        field = self.area_info()["sig_params_info"]
        length = len(field)
        if not verify(self.raw()[:field.offset()], self.raw()[field.offset():field.offset() + length]):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

class UBootCodeInfo(ImageArea):
    def __init__(self):
        structure_length = 0x400
        signature_length = 0x180
        fields = [
                FieldLe("image_id"),
                FieldLe("structure_version"),
                FieldLe("structure_length"),
                FieldLe("signature_length"),
                FieldLe("uboot_version_ext"),
                FieldLe("mask_uboot_version_ext"),
                FieldLe("msid_ext"),
                FieldLe("mask_msid_ext"),
                FieldLe("uboot_code_area_addr"),
                FieldLe("uboot_code_area_len"),
                Field("uboot_code_area_hash", 32),
                FieldLe("uboot_enc_flag"),
                Field("key_material_uboot", 16),
                Field("reserved1", 16),
                Field("iv", 16),
                FieldLe("uboot_compress_flag"),
                FieldLe("uboot_uncompressed_len"),
                FieldLe("text_segment_size"),
                FieldLe("uboot_entry_point"),
                Field("reserved2", structure_length - 140 - signature_length),
                Field("sig_uboot_info", signature_length),
                ]
        super(UBootCodeInfo, self).__init__("ree_boot_info_area", fields)

    def check(self, dec_enable, verify):
        static_info = {
                "image_id" : 0x4bf01e2d,
                "structure_version": 0x00010000,
                "structure_length": 0x400,
                "signature_length": 0x180,
                "uboot_code_area_addr": 0x0,
                "uboot_compress_flag": 0x0,
                }

        if not super(UBootCodeInfo, self).check(static_info):
            return False

        if dec_enable:
            if self.area_info()["uboot_enc_flag"].val() == 0x3c7896e1:
                log.error("{}: Enc_Flag check fail: dec_enable otp has burned, but it is a plaintext image.".format(self.name()))
                return False

        field = self.area_info()["sig_uboot_info"]
        length = len(field)
        raw = self.raw()
        if not verify(raw[:field.offset()], raw[field.offset():field.offset() + length]):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True


class CodeArea(ImageArea):
    def __init__(self, length, name):
        fields = [
                Field("Code", length),
                ]

        super(CodeArea, self).__init__(name, fields)

    def dump(self):
        log.info("========== {} ==========".format(self.name()))
        for k, v in self.area_info().items():
            if k == "length":
                log.info("area length: {:#x}".format(v))
        log.info("=========== end =========")
        log.info("")


class DecTool:
    def __init__(self, kdf, area_tool):
        self.__area_tool = area_tool
        self.__kdf = kdf

    def update(self, key_type, key_material, iv, rootkey):
        self.__key_type = key_type
        self.__key_material = key_material
        self.__iv = iv
        self.__oem_root_key = rootkey

    def dec(self, data):
        content_key = []
        if self.__key_type == 'TEE_GSL':
            content_key = self.__kdf.derive_keys(key_type='TEE', key_id="2a13c812", material=self.__key_material, rootkey=self.__oem_root_key)
        elif self.__key_type == 'REE_GSL':
            content_key = self.__kdf.derive_keys(key_type='REE', key_id="2a13c812", material=self.__key_material, rootkey=self.__oem_root_key)
        elif self.__key_type == 'UBOOT':
            content_key = self.__kdf.derive_keys(key_type='REE', key_id="2a13c823", material=self.__key_material, rootkey=self.__oem_root_key)

        return self.__area_tool.enc_cbc(data, content_key[:16], self.__iv, data_name='code data', decrypt=True)


class DUMMY:
    pass


def build_pubkey(data):
    return PublicKey(data[:384], data[384:388])

class ImageMap(object):
    def __init__(self, image):
        # start, len, area
        self.__image_map = [
                {'offset': '0x0',
                 'len': '0x200',
                 'area': RootPublicKeysArea(key_name="ree")},
                {'offset': '0x200',
                 'len': '0x200',
                 'area': RootPublicKeysArea(key_name="tee")},
                {'offset': '0x400',
                 'len':  '0x400',
                 'area': GSLKeyArea()},
                {'offset': '0x800',
                 'len':  '0x400',
                 'area': GSLCodeInfo()},
                {'offset': '0xC00',
                 'len':  'gsl_info_area.gsl_code_area_len.val()',
                 'area': CodeArea(0, 'gsl_code_area')},
                {'offset': '0xC00 + gsl_info_area.gsl_code_area_len.val()',
                 'len':  '0x400',
                 'area': REEBootKeyArea()},
                {'offset': '0x1000 + gsl_info_area.gsl_code_area_len.val()',
                 'len':  '0x400',
                 'area': ParamsAreaInfo()},
                {'offset': '0x1400 + gsl_info_area.gsl_code_area_len.val()',
                 'len':  'param_info_area.params_area_num.val() * param_info_area.params_area_len.val()',
                 'area': CodeArea(0, 'para_area')},
                {'offset': '0x1400 + gsl_info_area.gsl_code_area_len.val() + param_info_area.params_area_num.val() * param_info_area.params_area_len.val()',
                 'len':  '0x400',
                 'area': UBootCodeInfo()},
                {'offset': '0x1800 + gsl_info_area.gsl_code_area_len.val() + param_info_area.params_area_num.val() * param_info_area.params_area_len.val()',
                 'len':  'ree_boot_info_area.uboot_code_area_len.val()',
                 'area': CodeArea(0, 'uboot_area')},
        ]

        self.__image_map = dict(
                zip(map(lambda x: x['area'].name(), self.__image_map),
                    self.__image_map))

        self.__image = image

    def set_area(self, area_name, area):
        self.__image_map[area_name]['area'] = area

    def get_area(self, area_name):
        return self.__image_map[area_name]['area']

    def get_area_len(self, area_name):
        return self.get_val(self.__image_map[area_name]['len'])

    def get_area_offset(self, area_name):
        return self.get_val(self.__image_map[area_name]['offset'])

    def parse_area(self, area_name):
        image = self.__image
        self.get_area(area_name).parse(image[
            self.get_area_offset(area_name):
            self.get_area_offset(area_name)
            + self.get_area_len(area_name)])

    def check(self, oemcheck):
        self.parse_area('ree_rootkey')
        self.parse_area('tee_rootkey')
        self.parse_area('gsl_key_area')
        self.parse_area('gsl_info_area')
        self.parse_area('ree_boot_key_area')
        self.parse_area('param_info_area')
        self.parse_area('ree_boot_info_area')

        self.set_area('gsl_code_area', CodeArea(
            self.get_val('gsl_info_area.gsl_code_area_len.val()'),
            'gsl_code_area'))

        self.set_area('para_area', CodeArea(
            self.get_val('param_info_area.params_area_num.val()'
                         + '* param_info_area.params_area_len.val()'),
            'para_area'))

        self.set_area('uboot_area',
                      CodeArea(self.get_val(
                          'ree_boot_info_area.uboot_code_area_len.val()'),
                               'uboot_area'))

        self.parse_area('gsl_code_area')
        self.parse_area('para_area')
        self.parse_area('uboot_area')

        if not oemcheck.otp().ree_verify_enable() and \
            not self.get_area('ree_rootkey').area_info()["verify_enable_ext"] == 0x2A13C812:
            return False

        if not self.get_area('ree_rootkey').check(oemcheck.otp().ree_verify_enable(),
                                                  lambda x:
                                                  oemcheck.area_tool()
                                                  .digest(x),
                                                  oemcheck.otp()
                                                  .ree_hash_flash_root_key()):
            return False

        ree_rootpubkey = build_pubkey(self
                                      .get_area('ree_rootkey')
                                      .area_info()['flash_root_public_key']
                                      .val())

        if not self.get_area('gsl_key_area').check(
                lambda data, sign:
                oemcheck.area_tool()
                .verify(ree_rootpubkey, data, sign)):
            return False

        gsl_external_public_key = build_pubkey(
                self.get_area('gsl_key_area')
                .area_info()['gsl_external_public_key'].val())

        if not self.get_area('gsl_info_area').check(
                oemcheck.otp().ree_dec_enable(), lambda data, sign:
                oemcheck.area_tool()
                .verify(gsl_external_public_key, data, sign)):
            return False

        dectool = oemcheck.dectool()
        dectool.update('REE_GSL',
                       self.get_area('gsl_info_area').area_info()['key_material_gsl'].val(),
                       self.get_area('gsl_info_area').area_info()['iv'].val(),
                       oemcheck.otp().obfu_mrk1())

        decgsl = lambda data: dectool.dec(data) if self.get_area('gsl_info_area').area_info()['gsl_enc_flag'].val() != 0x3c7896e1 else data

        gsl_code_area_hash = self.get_area('gsl_info_area').area_info()['gsl_code_area_hash'].val()
        cal_gsl_code_area_hash = oemcheck.area_tool().digest(decgsl(self.get_area('gsl_code_area').raw()))
        if cal_gsl_code_area_hash != gsl_code_area_hash:
            log.error("gsl_code_area: hash check fail")
            return False
        else:
            log.info("gsl_code_area verify pass!!!")

        if not self.get_area('ree_boot_key_area').check(
                lambda data, sign: oemcheck.area_tool().verify(ree_rootpubkey, data, sign)):
            return False

        ree_boot_external_public_key = build_pubkey(self.get_area('ree_boot_key_area').area_info()['ree_boot_external_public_key'].val())

        self.get_area('param_info_area').check(
            lambda data, sign: oemcheck.area_tool().verify(ree_boot_external_public_key, data, sign))

        board_index_table = [self.get_val('param_info_area.board_index_hash_table{}.val()'.format(ind)) for ind in range(8)]
        for ind in range(8):
            if board_index_table[ind] != 0xff:
                len = self.get_val('param_info_area.params_area_len.val()')
                hash = self.get_val('param_info_area.params_area_hash{}.val()'.format(ind))
                offset = ind * len
                cal_hash = oemcheck.area_tool().digest(self.get_area('para_area').raw()[offset:offset + len])
                if hash != cal_hash:
                    log.info('params_area_{} verify fail!!!'.format(ind))
                    return False
                else:
                    log.info('params_area_{} verify pass!!!'.format(ind))

        if not self.get_area('ree_boot_info_area').check(oemcheck.otp().ree_dec_enable(), lambda data, sign:
                                                         oemcheck.area_tool().verify(ree_boot_external_public_key,
                                                                                     data, sign)):
            return False

        dectool.update('UBOOT',
                       self.get_area('ree_boot_info_area').area_info()['key_material_uboot'].val(),
                       self.get_area('ree_boot_info_area').area_info()['iv'].val(),
                       oemcheck.otp().obfu_mrk1())

        ubootdec = lambda data: dectool.dec(data) if self.get_area('ree_boot_info_area').area_info()['uboot_enc_flag'].val() != 0x3c7896e1 else data

        uboot_code_area_hash = self.get_area('ree_boot_info_area').area_info()['uboot_code_area_hash'].val()
        cal_uboot_code_area_hash = oemcheck.area_tool().digest(ubootdec(self.get_area('uboot_area').raw()))
        if cal_uboot_code_area_hash != uboot_code_area_hash:
            log.error("uboot_area: hash check fail")
            return False
        else:
            log.info("reeboot_code_area verify pass!!!")

    def get_val(self, str):
        for k in self.__image_map.keys():
            vars()[k] = DUMMY()
            eval('vars(' + k + ')').update(self.get_area(k).area_info())

        return eval(str)


class OemChecker:
    def __init__(self, otp_file):
        from kdf_tools import KdfTool

        self.__otp = OneTimeProgrammable(otp_file)
        self.__area_tool = AreaTool('RSA3072+SHA256+AES')
        self.__dectool = DecTool(KdfTool(), self.__area_tool)

    def otp(self):
        return self.__otp

    def dectool(self):
        return self.__dectool

    def area_tool(self):
        return self.__area_tool

    def check(self, boot_image, tee_image=None):
        image_bytes = util.read_file(boot_image)
        image = ImageMap(image_bytes)
        image.check(self)

def tips(script):
    print('\nUsage: $ python3 %s OPTION' % script)
    print('OPTION:')
    print('  check     Check images using the input OTP values.')
    print('Examples:')
    print('  $ python3 %s check oem/otp_check.json image/oem/boot_image.bin image/oem/tee_image.bin' % script)
    print('')

if __name__ == "__main__":
    if len(sys.argv) == 3:
        otp_file, boot_image = sys.argv[1:]
        tee_image = None
    elif len(sys.argv) == 4:
        otp_file, boot_image, tee_image = sys.argv[1:]
    else:
        tips(sys.argv[0])
        sys.exit(1)
    otp_file, boot_image, tee_image = [sys.argv[1:], None] if len(sys.argv) < 3 else sys.argv[1:4]
    checker = OemChecker(otp_file)
    checker.check(boot_image, tee_image)
    log.info('Done')
