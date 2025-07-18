## Generating

To generate necessary binaries you need to build and run
[Enrollment app](https://github.com/3mdeb/UC1.1-Manifest#) from
[this repo](https://github.com/3mdeb/UC1.1-Manifest). If done properly, the
activation code and intrinsic key binaries will be generated.

```log
(...)
Activation code hex saved to /tmp/activation_code.hex
Activation code bin saved to /tmp/activation_code.bin
Intrinsic key hex saved to /tmp/intrinsic_key.hex
Intrinsic key bin saved to /tmp/intrinsic_key.bin
renamed '/tmp/activation_code.bin' -> '/home/user/UC1.1-Manifest/build/enrollment_data/activation_code.bin'
renamed '/tmp/activation_code.hex' -> '/home/user/UC1.1-Manifest/build/enrollment_data/activation_code.hex'
renamed '/tmp/intrinsic_key.bin' -> '/home/user/UC1.1-Manifest/build/enrollment_data/intrinsic_key.bin'
renamed '/tmp/intrinsic_key.hex' -> '/home/user/UC1.1-Manifest/build/enrollment_data/intrinsic_key.hex'
Output moved to build/enrollment_data/
```
Copy the generated `.bin` files here and rename them as specified in
[legend](#legend).

Note: This method is valid only for `x86` build systems due to tools like
`LinkServer` being `x86` only. If you're running an `ARM` based host, you'll
need to flash the enrollment app, capture the serial output and paste it to
[fallback_capture_enroll](https://github.com/3mdeb/UC1.1-Manifest/blob/main/scripts/fallback_capture_enroll.lua)
script.

## Legend

`ac` - activation code.  
`k` - intrinsic key.  
`_<number>` - asset number of the target platform, check the sticker.  
