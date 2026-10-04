from Crypto.Protocol.KDF import PBKDF2
from Crypto.Hash import SHA256
import binascii

import sys
import os

SCRIPT_DIR = os.path.dirname(__file__)
sys.path.append(SCRIPT_DIR)

import common.logger as log

class KdfTool:
    def derive_keys(self, key_type, key_id, material, rootkey):
        if key_type == "TEE":
            key_static = binascii.unhexlify('d1feef44')
        elif key_type == "REE":
            key_static = binascii.unhexlify('d1feef74')
        else:
            log.err('derive keys error!!!!')
            return None

        pad = binascii.unhexlify('00' * 8)
        salt = key_static + binascii.unhexlify(key_id) + pad + material[::-1]
        # pad = binascii.unhexlify('00' * 12)
        # salt = key_static + pad + material[::-1]
        salt = salt[::-1]
        passwd = rootkey
        keys = PBKDF2(passwd, salt, 32, count=3, hmac_hash_module=SHA256)
        return keys
