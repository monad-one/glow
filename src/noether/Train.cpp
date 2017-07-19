#include "noether/Train.h"
#include "noether/Tensor.h"

using namespace noether;

void TrainableData::train(const TrainingConfig &config) {
  size_t batchSize = config.batchSize;
  float L1Decay = config.L1Decay;
  float L2Decay = config.L2Decay;
  float learningRate = config.learningRate;
  float momentum = config.momentum;

  // Do not change the weights of input layers that are marked as untrainable.
  if (!isTrainable_)
    return;

  /// If we are using the momentum technique then we need to allocate an array
  /// for the gradient sum.
  if (momentum > 0.0 && gsum_.size() == 0) {
    gsum_.reset(weight_.dims());
  }

  auto sz = weight_.size();

  // For each weight/gradient pair:
  for (size_t x = 0; x < sz; x++) {
    // Do a simple SGD update:
    FloatTy L1Grad = L1Decay * (weight_.at(x) > 0 ? 1 : -1);
    FloatTy L2Grad = L2Decay * (weight_.at(x));
    FloatTy gij = (L2Grad + L1Grad + gradient_.at(x)) / batchSize;

    // Use the momentum to improve the gradient descent:
    // http://ufldl.stanford.edu/tutorial/supervised/OptimizationStochasticGradientDescent/
    if (momentum > 0.0) {
      // Momentum update:
      FloatTy dx = momentum * gsum_.at(x) - learningRate * gij;
      // Save this value for the next iteration:
      gsum_.at(x) = dx;
      // Apply the gradient.
      weight_.at(x) += dx;
    } else {
      // Use regular SGD:
      weight_.at(x) -= learningRate * gij;
    }
  }
}

void TrainableData::dump() {
  weight_.dump("W");
  if (gradient_.size())
    gradient_.dump("G", "\n");
  if (gsum_.size())
    gsum_.dump("Gsum", "\n");
}

void TrainableData::verify() {
  if (gradient_.size()) {
    assert(gradient_.size() == weight_.size() &&
           "Gradient tensor does not match weight tensor");
  }
  if (gsum_.size()) {
    assert(gsum_.size() == weight_.size() &&
           "Gradient tensor does not match weight tensor");
  }
}