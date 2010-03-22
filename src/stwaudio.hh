#ifndef STWAUDIO_HH
#define STWAUDIO_HH

/*-------- begin sfidl generated code --------*/



#include <bse/bsecxxplugin.hh>


/* enum prototypes */


/* choice prototypes */


/* record prototypes */
namespace Stw {
namespace Codec {
class AudioBlock;
typedef Sfi::RecordHandle<AudioBlock> AudioBlockHandle;
#define STW_CODEC_TYPE_AUDIO_BLOCK		BSE_CXX_DECLARED_RECORD_TYPE (Stw::Codec, AudioBlock)
class Audio;
typedef Sfi::RecordHandle<Audio> AudioHandle;
#define STW_CODEC_TYPE_AUDIO		BSE_CXX_DECLARED_RECORD_TYPE (Stw::Codec, Audio)


/* sequence prototypes */
class AudioBlockSeq;
#define STW_CODEC_TYPE_AUDIO_BLOCK_SEQ		BSE_CXX_DECLARED_SEQUENCE_TYPE (Stw::Codec, AudioBlockSeq)


/* class prototypes */


/* enum definitions */


/* sequence definitions */
class AudioBlockSeq : public Sfi::Sequence< ::Stw::Codec::AudioBlockHandle > {
public:
  AudioBlockSeq (unsigned int n = 0) : Sfi::Sequence< ::Stw::Codec::AudioBlockHandle > (n) {}
  static inline ::Stw::Codec::AudioBlockSeq from_seq (SfiSeq *seq);
  static inline SfiSeq *to_seq (const ::Stw::Codec::AudioBlockSeq & seq);
  static inline ::Stw::Codec::AudioBlockSeq value_get_boxed (const GValue *value);
  static inline void value_set_boxed (GValue *value, const ::Stw::Codec::AudioBlockSeq & self);
  static inline const char* options   () { return ""; }
  static inline const char* blurb     () { return ""; }
  static inline const char* authors   () { return ""; }
  static inline const char* license   () { return ""; }
  static inline const char* type_name () { return "StwCodecAudioBlockSeq"; }
  static inline GParamSpec* get_element ();
};



/* record definitions */
class AudioBlock : public ::Sfi::GNewable {
public:
  Sfi::FBlock meaning;
  Sfi::FBlock freqs;
  Sfi::FBlock mags;
  Sfi::FBlock phases;
  Sfi::FBlock original_fft;
  static inline ::Stw::Codec::AudioBlockHandle from_rec (SfiRec *rec);
  static inline SfiRec *to_rec (const ::Stw::Codec::AudioBlockHandle & ptr);
  static inline ::Stw::Codec::AudioBlockHandle value_get_boxed (const GValue *value);
  static inline void value_set_boxed (GValue *value, const ::Stw::Codec::AudioBlockHandle & self);
  static inline const char* options   () { return ""; }
  static inline const char* blurb     () { return ""; }
  static inline const char* authors   () { return ""; }
  static inline const char* license   () { return ""; }
  static inline const char* type_name () { return "StwCodecAudioBlock"; }
  static inline SfiRecFields get_fields ();
};

class Audio : public ::Sfi::GNewable {
public:
  Sfi::Real mix_freq;
  Sfi::Real frame_size_ms;
  Sfi::Real frame_step_ms;
  Sfi::Real fundamental_freq;
  Sfi::Int zeropad;
  ::Stw::Codec::AudioBlockSeq contents;
  static inline ::Stw::Codec::AudioHandle from_rec (SfiRec *rec);
  static inline SfiRec *to_rec (const ::Stw::Codec::AudioHandle & ptr);
  static inline ::Stw::Codec::AudioHandle value_get_boxed (const GValue *value);
  static inline void value_set_boxed (GValue *value, const ::Stw::Codec::AudioHandle & self);
  static inline const char* options   () { return ""; }
  static inline const char* blurb     () { return ""; }
  static inline const char* authors   () { return ""; }
  static inline const char* license   () { return ""; }
  static inline const char* type_name () { return "StwCodecAudio"; }
  static inline SfiRecFields get_fields ();
};



/* enum declarations */


/* sequence type declarations */
BSE_CXX_DECLARE_SEQUENCE (AudioBlockSeq);



/* record type declarations */
BSE_CXX_DECLARE_RECORD (AudioBlock);

BSE_CXX_DECLARE_RECORD (Audio);



/* procedure prototypes */


/* class definitions */


/* choice implementations */


/* record implementations */
::Stw::Codec::AudioBlockHandle
AudioBlock::from_rec (SfiRec *sfi_rec)
{
  GValue *element;

  if (!sfi_rec)
    return Sfi::INIT_NULL;

  ::Stw::Codec::AudioBlockHandle rec = Sfi::INIT_DEFAULT;
  element = sfi_rec_get (sfi_rec, "meaning");
  if (element)
    rec->meaning = ::Sfi::FBlock::value_get_fblock (element);
  element = sfi_rec_get (sfi_rec, "freqs");
  if (element)
    rec->freqs = ::Sfi::FBlock::value_get_fblock (element);
  element = sfi_rec_get (sfi_rec, "mags");
  if (element)
    rec->mags = ::Sfi::FBlock::value_get_fblock (element);
  element = sfi_rec_get (sfi_rec, "phases");
  if (element)
    rec->phases = ::Sfi::FBlock::value_get_fblock (element);
  element = sfi_rec_get (sfi_rec, "original_fft");
  if (element)
    rec->original_fft = ::Sfi::FBlock::value_get_fblock (element);
  return rec;
}

SfiRec *
AudioBlock::to_rec (const ::Stw::Codec::AudioBlockHandle & rec)
{
  SfiRec *sfi_rec;
  GValue *element;

  if (!rec)
    return NULL;

  sfi_rec = sfi_rec_new ();
  element = sfi_rec_forced_get (sfi_rec, "meaning", SFI_TYPE_FBLOCK);
  ::Sfi::FBlock::value_set_fblock (element, rec->meaning);
  element = sfi_rec_forced_get (sfi_rec, "freqs", SFI_TYPE_FBLOCK);
  ::Sfi::FBlock::value_set_fblock (element, rec->freqs);
  element = sfi_rec_forced_get (sfi_rec, "mags", SFI_TYPE_FBLOCK);
  ::Sfi::FBlock::value_set_fblock (element, rec->mags);
  element = sfi_rec_forced_get (sfi_rec, "phases", SFI_TYPE_FBLOCK);
  ::Sfi::FBlock::value_set_fblock (element, rec->phases);
  element = sfi_rec_forced_get (sfi_rec, "original_fft", SFI_TYPE_FBLOCK);
  ::Sfi::FBlock::value_set_fblock (element, rec->original_fft);
  return sfi_rec;
}

::Stw::Codec::AudioBlockHandle
AudioBlock::value_get_boxed (const GValue *value)
{
  return ::Stw::Codec::AudioBlockHandle::value_get_boxed (value);
}

void
AudioBlock::value_set_boxed (GValue *value, const ::Stw::Codec::AudioBlockHandle & self)
{
  ::Stw::Codec::AudioBlockHandle::value_set_boxed (value, self);
}

SfiRecFields
AudioBlock::get_fields()
{
  static SfiRecFields rfields = { 0, NULL };
  if (!rfields.n_fields)
    {
      static GParamSpec *fields[5 + 1];
      rfields.n_fields = 5;
      fields[0] = sfidl_pspec_FBlock_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2640,"meaning");
      fields[1] = sfidl_pspec_FBlock_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2641,"freqs");
      fields[2] = sfidl_pspec_FBlock_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2642,"mags");
      fields[3] = sfidl_pspec_FBlock_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2643,"phases");
      fields[4] = sfidl_pspec_FBlock_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2646,"original_fft");
      rfields.fields = fields;
    }
  return rfields;
}
::Stw::Codec::AudioHandle
Audio::from_rec (SfiRec *sfi_rec)
{
  GValue *element;

  if (!sfi_rec)
    return Sfi::INIT_NULL;

  ::Stw::Codec::AudioHandle rec = Sfi::INIT_DEFAULT;
  element = sfi_rec_get (sfi_rec, "mix_freq");
  if (element)
    rec->mix_freq = sfi_value_get_real (element);
  element = sfi_rec_get (sfi_rec, "frame_size_ms");
  if (element)
    rec->frame_size_ms = sfi_value_get_real (element);
  element = sfi_rec_get (sfi_rec, "frame_step_ms");
  if (element)
    rec->frame_step_ms = sfi_value_get_real (element);
  element = sfi_rec_get (sfi_rec, "fundamental_freq");
  if (element)
    rec->fundamental_freq = sfi_value_get_real (element);
  element = sfi_rec_get (sfi_rec, "zeropad");
  if (element)
    rec->zeropad = sfi_value_get_int (element);
  element = sfi_rec_get (sfi_rec, "contents");
  if (element)
    rec->contents = ::Stw::Codec::AudioBlockSeq::value_get_boxed (element);
  return rec;
}

SfiRec *
Audio::to_rec (const ::Stw::Codec::AudioHandle & rec)
{
  SfiRec *sfi_rec;
  GValue *element;

  if (!rec)
    return NULL;

  sfi_rec = sfi_rec_new ();
  element = sfi_rec_forced_get (sfi_rec, "mix_freq", SFI_TYPE_REAL);
  sfi_value_set_real (element, rec->mix_freq);
  element = sfi_rec_forced_get (sfi_rec, "frame_size_ms", SFI_TYPE_REAL);
  sfi_value_set_real (element, rec->frame_size_ms);
  element = sfi_rec_forced_get (sfi_rec, "frame_step_ms", SFI_TYPE_REAL);
  sfi_value_set_real (element, rec->frame_step_ms);
  element = sfi_rec_forced_get (sfi_rec, "fundamental_freq", SFI_TYPE_REAL);
  sfi_value_set_real (element, rec->fundamental_freq);
  element = sfi_rec_forced_get (sfi_rec, "zeropad", SFI_TYPE_INT);
  sfi_value_set_int (element, rec->zeropad);
  element = sfi_rec_forced_get (sfi_rec, "contents", SFI_TYPE_SEQ);
  ::Stw::Codec::AudioBlockSeq::value_set_boxed (element, rec->contents);
  return sfi_rec;
}

::Stw::Codec::AudioHandle
Audio::value_get_boxed (const GValue *value)
{
  return ::Stw::Codec::AudioHandle::value_get_boxed (value);
}

void
Audio::value_set_boxed (GValue *value, const ::Stw::Codec::AudioHandle & self)
{
  ::Stw::Codec::AudioHandle::value_set_boxed (value, self);
}

SfiRecFields
Audio::get_fields()
{
  static SfiRecFields rfields = { 0, NULL };
  if (!rfields.n_fields)
    {
      static GParamSpec *fields[6 + 1];
      rfields.n_fields = 6;
      fields[0] = sfidl_pspec_Real_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2654,"mix_freq");
      fields[1] = sfidl_pspec_Real_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2655,"frame_size_ms");
      fields[2] = sfidl_pspec_Real_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2656,"frame_step_ms");
      fields[3] = sfidl_pspec_Real_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2657,"fundamental_freq");
      fields[4] = sfidl_pspec_Int_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2658,"zeropad");
      fields[5] = sfidl_pspec_Sequence_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2659, "contents", Stw::Codec::AudioBlockSeq::get_element());
      rfields.fields = fields;
    }
  return rfields;
}


/* sequence implementations */
::Stw::Codec::AudioBlockSeq
AudioBlockSeq::from_seq (SfiSeq *sfi_seq)
{
  ::Stw::Codec::AudioBlockSeq cseq;
  guint i, length;

  if (!sfi_seq)
    return cseq;

  length = sfi_seq_length (sfi_seq);
  cseq.resize (length);
  for (i = 0; i < length; i++)
    {
      GValue *element = sfi_seq_get (sfi_seq, i);
      cseq[i] = ::Stw::Codec::AudioBlock::value_get_boxed (element);
    }
  return cseq;
}

SfiSeq *
AudioBlockSeq::to_seq (const ::Stw::Codec::AudioBlockSeq & cseq)
{
  SfiSeq *sfi_seq = sfi_seq_new ();
  for (guint i = 0; i < cseq.length(); i++)
    {
      GValue *element = sfi_seq_append_empty (sfi_seq, SFI_TYPE_REC);
      ::Stw::Codec::AudioBlock::value_set_boxed (element, cseq[i]);
    }
  return sfi_seq;
}

::Stw::Codec::AudioBlockSeq
AudioBlockSeq::value_get_boxed (const GValue *value)
{
  return ::Sfi::cxx_value_get_boxed_sequence< AudioBlockSeq> (value);
}

void
AudioBlockSeq::value_set_boxed (GValue *value, const ::Stw::Codec::AudioBlockSeq & self)
{
  ::Sfi::cxx_value_set_boxed_sequence< AudioBlockSeq> (value, self);
}

GParamSpec*
AudioBlockSeq::get_element()
{
  static GParamSpec *element = NULL;
  if (!element)
    element = sfidl_pspec_Record_default (NULL,"/home/stefan/src/spectmorph/src/stwaudio.idl",2650, "block", Stw::Codec::AudioBlock::get_fields());
  return element;
}



/* class implementations */


/* procedure implementations */


/*  type registrations */
} // Codec
} // Stw

/*-------- end sfidl generated code --------*/


#endif
