#include "NodeDelegateModelRegistry.hpp"

using QtNodes::NodeDelegateModel;
using QtNodes::NodeDelegateModelRegistry;

std::unique_ptr<NodeDelegateModel> NodeDelegateModelRegistry::create(QString const &modelName)
{
    auto it = _registeredItemCreators.find(modelName);

    if (it != _registeredItemCreators.end()) {
        return it->second();
    }

    return nullptr;
}

QString NodeDelegateModelRegistry::categoryDisplayName(QString const &category) const
{
    auto it = _categoriesDisplayName.find(category);

    if (it != _categoriesDisplayName.end()) {
        return it->second;
    }

    return category;
}
