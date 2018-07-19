
class TLVNode:
    def __init__(self, tlength=2, llength=2, byteorder='big'):
        self._tlength = tlength
        self._llength = llength
        self._byteorder = byteorder
        self._t = bytes(tlength)
        self._l = bytes(llength)
        self._v = None

    def byteorder(self):
        '''
        returns 'big', 'little'
        '''
        return self._byteorder

    def tlength(self):
        return self._tlength

    def llength(self):
        return self._llength

    def t(self):
        '''
        returns tag bytes
        '''
        return self._t

    def sett(self, tagbytes=None, tagint=None, taghex=None):
        tlength = self.tlength()

        ta = bytearray(tlength)

        if tagbytes is not None:
            r = min(tlength, len(tagbytes))
            for i in range(r):
                ta[i] = tagbytes[i]
        elif tagint is not None:
            ta = bytearray(tagint.to_bytes(tlength, self.byteorder()))
        elif taghex is not None:
            tagbytes = bytes.fromhex(taghex)
            r = min(tlength, len(tagbytes))
            for i in range(r):
                ta[i] = tagbytes[i]
        else:
            pass

        self._t = bytes(ta)

    def l(self):
        '''
        returns the int value in length bytes
        '''
        return int.from_bytes(self._l, self.byteorder())

    def lbytes(self):
        return self._l

    def setl(self, lengthbytes=None, lengthint=None, lengthhex=None):
        llength = self.llength()
        la = bytearray(llength)

        if lengthbytes is not None:
            r = min(llength, len(lengthbytes))
            for i in range(r):
                la[i] = lengthbytes[i]
        elif lengthint is not None:
            la = bytearray(lengthint.to_bytes(llength, self.byteorder()))
        elif lengthhex is not None:
            lengthbytes = bytes.fromhex(lengthhex)
            r = min(llength, len(lengthbytes))
            for i in range(r):
                la[i] = lengthbytes[i]
        else:
            pass

        self._l = bytes(la)

    def v(self):
        '''
        returns the v bytes
        '''
        return self._v

    def setv(self, vbytes=None, vhex=None, tlvnode=None):
        '''
        Notice: l would be changed accordingly
        '''
        ba = None

        if vbytes is not None:
            ba = bytearray(vbytes)
        elif vhex is not None:
            ba = bytearray.fromhex(vhex)
        elif tlvnode is not None:
            ba = bytearray(tlvnode.toBytes()) 
        else:
            ba = bytearray(0)

        length = len(ba)
        self.setl(lengthint=length)

        self._v = bytes(ba)

    def nodeLength(self):
        return self.tlength() + self.llength() + self.l()

    def toBytes(self):
        ba = bytearray()
        ba = ba + self.t()
        ba = ba + self.lbytes()
        ba = ba + self.v()

        return bytes(ba)

    def toHex(self, spliter=None):
        b = self.toBytes()
        if b is None:
            return None
        h = b.hex()

        if spliter is not None:
            hexcpy = str(h)
            h = ''
            hexlen = len(hexcpy)
            for i in range(hexlen):
                h = h + hexcpy[i]
                if i < hexlen - 1 and i % 2 != 0:
                    h = h + spliter

        return h

    def load(self, tlvbytes=None, tlvhex=None):
        t = None
        l = None
        v = None

        if tlvhex is not None:
            tlvbytes = bytes.fromhex(tlvhex)

        tlength = self.tlength()
        llength = self.llength()

        if len(tlvbytes) < tlength + llength:
            return False

        t = tlvbytes[0 : tlength]
        l = tlvbytes[tlength : tlength + llength]
        v = tlvbytes[tlength + llength : len(tlvbytes)]

        lv = int.from_bytes(l, byteorder=self.byteorder())
        if lv != len(v):
            return False

        self.sett(t)
        self.setl(l)
        self.setv(v)

        return True




class TLV (TLVNode):
    def __init__(self, tlength=2, llength=2, byteorder='big'):
        TLVNode.__init__(self, tlength, llength, byteorder)
        self._bytes = bytesdata
        if hexstr is not None:
            self._bytes = bytes.fromhex(tlvstr)







if __name__ == '__main__':
    pass