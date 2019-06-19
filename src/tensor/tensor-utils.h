// tensor/tensor-utils.h

// Copyright      2019  Johns Hopkins University (author: Daniel Povey)

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#ifndef KALDI_TENSOR_TENSOR_UTILS_H_
#define KALDI_TENSOR_TENSOR_UTILS_H_ 1


#include "tensor/tensor-impl.h"
#include "tensor/pattern-utils.h"
#include "tensor/tensor.h"

namespace kaldi {
namespace tensor {


/**
  This function returns true if a and b have the same dtype
  and device.  See also Broadcastable().
*/
inline bool Compatible(const Tensor &a, const Tensor &b) {
  return Compatible(*a.impl_, *b.impl_);
}

/**
  This function returns true if the Patterns of a and b are
  broadastable.
*/
inline bool Broadcastable(const Tensor &a, const Tensor &b) {
  return Broadcastable(*a.impl_, *b.impl_);
}


/**
  This function returns true if a and b have the same dtype
  and device and are broadcastable; equivalent to
  `Broadcastable(a, b) && Compatible(a, b)`.
*/
inline bool BroadcastablAendCompatible(const Tensor &a, const Tensor &b) {
  return Compatible(*a.impl_, *b.impl_) &&
      Broadcastable(*a.impl_, *b.impl_);
}


inline bool Overlap(const Tensor &a, const Tensor &b) {
  return a.impl_->storage.get() == b.impl.storage.get() &&
      PatternsOverlap(a.impl_->pattern, b.impl_->pattern);
}


/**
   Returns true if the Tensor t covers its entire allocated storage region,
   meaning every byte of the storage region is accessible through t.
*/
inline bool IsWhole(const Tensor &t) {
  return IsWhole(*t.impl_);
}

/*
  This function returns true if a, b and c have the same dtype
  and device; equivalent to Compatible(a, b) && Compatible(b, c).
*/
inline bool Compatible(const TensorImpl &a, const TensorImpl &b,
                       const TensorImpl &c) {
  return Compatible(*a.impl_, *b.impl_, *c.impl_);
}


/**  This function returns true if the dimensions of tensor patterns
     a and b are broadcastable in the PyTorch sense.  What this means
     for tensors with the same num-axes is that dims for axis i
     must either be the same or one of them must be 1.  For tensors
     with different num-axes we (conceptually) check this after
     padding with leading (dim=1)'s; for
     instance, dims=[2,8,3] and dims=[8,1] would be broadcastable because
     the [8,1] would be interpreted as [1,8,1].  (The examples above
     are in the public ordering, not the reversed private ordering.)

     If 'b_non_reducing' is true, then we do not allow any dim of
     b to be 1 where the corresponding dim of a was not 1.
 */
inline bool Broadcastable(const Tensor &a, const Tensor &b,
                          bool b_non_reducing = false) {
  return Broadcastable(a.impl_.pattern, b.impl_.pattern,
                       b_non_reducing);
}

/**  This function returns true if the dimensions of Tensors
     a, b and c are broadcastable in the PyTorch sense (meaning;
     after padding their dims on the left with ones to make them
     have the same num-axes, corresponding dimensions are either
     identical or 1).  See the version of Broadcastable() above
     for more information.

       @param [in] a  The first Tensor
       @param [in] b  The second Tensor
       @param [in] c  The third Tensor
       @param [in] c_non_reducing   If true, then we do not allow a dim of
                      c to be 1 while corresponding dims of a or b
                      are > 1.
 */
inline bool Broadcastable(const Tensor &a, const Tensor &b,
                          const Tensor &c, bool c_non_reducing = false) {
  return Broadcastable(a.impl_.pattern, b.impl_.pattern,
                       c.impl_.pattern, c_non_reducing);
}

/**
   Returns true if the 'dims' vectors of a and b are the same.
   Does not require the number of axes to be the same, so effectively
   it's testing that the dims are the same after padding on the left
   with dim=1 (here referring to the public, non-reversed numbering
   of the dims).

   This is a stronger condition than Broadcastable(a, b).
 */
inline bool SameDim(const Tensor &a, const Tensor &b) {
  return SameDim(a.impl_.pattern, b.impl_.pattern);
}

/**
   Returns true if the 'dims' vectors of a, b and c are all the same.
   Does not require the number of axes to be the same, so effectively
   it's testing that the dims are the same after padding on the left
   with dim=1 (here referring to the public, non-reversed numbering
   of the dims).

   This is a stronger condition than Broadcastable(a, b, c).
 */
inline bool SameDim(const Tensor &a, const Tensor &b,
                    const Tensor &c) {
  return SameDim(a.impl_.pattern, b.impl_.pattern);
}

inline void CheckUnchangedSince(int64 tick, const Tensor &a) {
  // TODO.  Access its storage and check not changed since then.
}


/**
   This is to be called from any routine that writes to the memory underlying a
   Tensor; in debug mode it registers that the Tensor has been changed, which
   will later be used to check that the preconditions of the autograd framework
   (in terms of in-place operations) are satisfied.
 */
inline void RegisterTensorChange(const Tensor &a) {
  RegisterTensorChange(*a.impl_);
}


/**
   Returns the number of elements in the Tensor, which equals the product of its
   dimensions, i.e. the product from `axis = 0 ... a.NumAxes() - 1`, of
   `a.Dim(axis)`.
 */
inline int64 NumElements(const Tensor &a) {
  return NumElements(*a.impl_);
}

/**
   This is the Tensor-level version of CanonicalizePattern() from
   pattern-utils.h.  It ensures that the Tensor's pattern is canonical.
   If this changes the Pattern, this will involve allocating a new
   TensorImpl (since we always assume that TensorImpl's may be shared
   by other Tensors).
*/
void CanonicalizeTensor(Tensor *tensor);

/**
   This is the Tensor-level version of CompressPatterns() from pattern-utils.h.
   It ensures that the Tensors
 */
void CompressTensors(ArrayRef<Tensor*> tensors);


/**
   Returns a Tensor referencing a new TensorImpl; it will be as t except the
   pattern will be the one provided.
 */
Tensor WithPattern(const Tensor &t, const Pattern &pattern);


/**
   This is to be called when any operation makes use of the memory underlying a
   Tensor.
      kRead
      kWrite
      kReadWrite
      kReadInvalidate
      kInvalidate
*/
inline void RecordUse(const Tensor &tensor,
                      TensorUseEnum use_type) {
  if (DebugMode()) {
    tensor.impl_->storage_->GetMemoryChecker()->RecordUse(
        SizeOf(impl.dtype), impl.pattern);
  }
}



// Implementation for 2-Tensor DebugNormalOp (see declaration below); called in
// debug mode only.
void DebugNormalOpInternal(const Tensor &a, TensorUseEnum a_use,
                           const Tensor &b, TensorUseEnum b_use);
// Implementation for 3-Tensor DebugNormalOp (see declaration below); called in
// debug mode only.
void DebugNormalOpInternal(const Tensor &a, TensorUseEnum a_use,
                           const Tensor &b, TensorUseEnum b_use,
                           const Tensor &c, TensorUseEnum c_use);



/**
   This convenience function is to be used in the implementation of
   Tensors (inside the Do() function).  In debug mode, it makes various
   checks.  This is for use in "normal" ops, i.e. ops that operate on
   the same data-types and on the same device.
   This version is for use in Ops that operate on two tensors.

      @param [in] a     The first Tensor the Op works on.
      @param [in] a_use The use-type of Tensor a,
                        saying what kind of operation we are
                        doing on it: one of
                         - kRead
                         - kWrite
                         - kReadWrite
                         - kReadInvalidate
                         - kInvalidate
                        (the ones with Invalidate may be relatively
                        rare; they are for Ops where we are avoiding
                        some operation in the expectation that the data
                        won't be used afterward).
      @param [in] b     The second Tensor the Op works on
      @param [in] b_use The use-type of Tensor b


*/
inline void DebugNormalOp(const Tensor &a, TensorUseEnum a_use,
                          const Tensor &b, TensorUseEnum b_use) {
  if (DebugMode())
    DebugNormalOpInternal(a, a_use, b, b_use);
}



/**
   This convenience function is to be used in the implementation of
   Tensors (inside the Do() function).  In debug mode, it makes various
   checks.  This is for use in "normal" ops, i.e. ops that operate on
   the same data-types and on the same device.
   This version is for use in Ops that operate on two tensors.

      @param [in] a     The first Tensor the Op works on.
      @param [in] a_use The use-type of Tensor a,
                        saying what kind of operation we are
                        doing on it: one of
                         - kRead
                         - kReadWrite
                         - kReadInvalidate
                         - kInvalidate
                        (the ones with Invalidate may be relatively
                        rare; they are for Ops where we are avoiding
                        some operation in the expectation that the data
                        won't be used afterward).
      @param [in] b     The second Tensor the Op works on
      @param [in] b_use The use-type of Tensor b
      @param [in] c     The second Tensor the Op works on
      @param [in] c_use The use-type of Tensor c
*/
inline void DebugNormalOp(const Tensor &a, TensorUseEnum a_use,
                          const Tensor &b, TensorUseEnum b_use,
                          const Tensor &c, TensorUseEnum c_use) {
  if (DebugMode())
    DebugNormalOpInternal(a, a_use, b, b_use, c, c_use);
}

/**
   Calling this ensures that when (in future) a Tensor's storage region is
   allocated, it will be zeroed.  This won't have any effect if the storage
   region was already allocated.  Note: storage regions are not allocated
   until they are actually used (e.g. by calling GetData()), so if Tensor
   'a' is freshly created, this will have an effect.
 */
inline void ZeroOnAllocation(const Tensor &a) {
  a.impl_->storage->ZeroOnAllocation();
}



}  // namespace tensor
}  // namespace kaldi


#endif  // KALDI_TENSOR_TENSOR_H_
