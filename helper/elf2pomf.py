# -*- coding: utf-8 -*-

from elftools.elf.elffile import ELFFile
from sys import argv
import pdb

INNAME=argv[1]
OUTNAME=argv[2]

f = open(INNAME, 'rb')
elf = ELFFile(f)

symtab = elf.get_section_by_name('.symtab')

# Get the header
hdr_section = elf.get_section_by_name('.rodata.POMF_HEAD')
header = hdr_section.data()

# Get the event track array
track_section = elf.get_section_by_name('.rodata.POMF_TUNE')

samples = {}

# Check for sample references
track_relocations = elf.get_section_by_name('.rela.rodata.POMF_TUNE')
if track_relocations is not None:
    # We have sample references in the track
    for reloc in track_relocations.iter_relocations():
        sym_no = reloc['r_info_sym']
        if sym_no in samples:
            samples[sym_no]['offsets'].append(reloc['r_offset'])
        else:
            # Load sample anew
            sample = { 
                'offsets': [reloc['r_offset']]
            }
            print("Relocation at ",reloc['r_offset'])
            sample_hdr_sym = symtab.get_symbol(sym_no)
            print("Sample header: ", sample_hdr_sym.name)
            sample_hdr_section = elf.get_section(sample_hdr_sym['st_shndx'])
            sample_hdr_data = sample_hdr_section.data()
            sample['header'] = sample_hdr_data
            print("Header section: ", sample_hdr_section.name)
            sample_reloc_section = elf.get_section_by_name('.rela'+sample_hdr_section.name)
            assert(sample_reloc_section.num_relocations() == 1)
            sample_data_sym = symtab.get_symbol(sample_reloc_section.get_relocation(0)['r_info_sym'])
            print("Data symbol:", sample_data_sym.name)
            sample_data_sect = elf.get_section(sample_data_sym['st_shndx'])
            sample['data'] = sample_data_sect.data()
            # NB: Fixing address of data in the header is the job of the loader
            samples[sym_no] = sample

track_data = bytearray(track_section.data())

f.close()

samples_indexed = []
cur_sample_index = 0
for (_ , s) in samples.items():
    samples_indexed.append(s)
    idx_le = cur_sample_index.to_bytes(4, byteorder = 'little')
    for offset in s['offsets']:
        track_data[offset] = idx_le[0]
        track_data[offset + 1] = idx_le[1]
        track_data[offset + 2] = idx_le[2]
        track_data[offset + 3] = idx_le[3]
    cur_sample_index += 1

outf = open(OUTNAME, 'wb')

outf.write(header)
for s in samples_indexed:
    outf.write(b'saMP')
    size_le = (len(s['header']) + len(s['data'])).to_bytes(4, byteorder = 'little')
    outf.write(size_le)
    outf.write(s['header'])
    outf.write(s['data'])
outf.write(b'tuNE')
size_le = (len(track_data)).to_bytes(4, byteorder = 'little')
outf.write(track_data)