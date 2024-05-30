#pragma once

#include "Export.hpp"
#include "NodeDelegateModel.hpp"

#include <QtCore/QString>

#include <functional>
#include <memory>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace QtNodes {

/// Class uses map for storing models (name, model)
class NODE_EDITOR_PUBLIC NodeDelegateModelRegistry
{
public:
    using RegistryItemPtr = std::unique_ptr<NodeDelegateModel>;
    using RegistryItemCreator = std::function<RegistryItemPtr()>;
    using RegisteredModelCreatorsMap = std::unordered_map<QString, RegistryItemCreator>;
    using RegisteredModelsCategoryMap = std::unordered_map<QString, QString>;
    using RegisteredModelsDisplayNameMap = std::unordered_map<QString, QString>;
    using CategoriesSet = std::set<QString>;
    using CategoriesDisplayNameMap = std::unordered_map<QString, QString>;

    NodeDelegateModelRegistry() = default;
    ~NodeDelegateModelRegistry() = default;

    NodeDelegateModelRegistry(NodeDelegateModelRegistry const &) = delete;
    NodeDelegateModelRegistry(NodeDelegateModelRegistry &&) = default;

    NodeDelegateModelRegistry &operator=(NodeDelegateModelRegistry const &) = delete;

    NodeDelegateModelRegistry &operator=(NodeDelegateModelRegistry &&) = default;

public:
    template<typename ModelType>
    void registerModel(RegistryItemCreator creator,
                       QString const &category = QStringLiteral("Nodes"))
    {
        QString const name = computeName<ModelType>(creator);
        if (!_registeredItemCreators.count(name)) {
            _categories.insert(category);
            _registeredModelsCategory[name] = category;
            _registeredModelsDisplayName[name] = computeDisplayName<ModelType>(creator);
            _registeredItemCreators[name] = std::move(creator);
        }
    }

    template<typename ModelType>
    void registerModel(QString const &category = QStringLiteral("Nodes"))
    {
        RegistryItemCreator creator = []() { return std::make_unique<ModelType>(); };
        registerModel<ModelType>(std::move(creator), category);
    }

    std::unique_ptr<NodeDelegateModel> create(QString const &modelName);

    RegisteredModelCreatorsMap const &registeredModelCreators() const
    {
        return _registeredItemCreators;
    }

    RegisteredModelsCategoryMap const &registeredModelsCategoryAssociation() const
    {
        return _registeredModelsCategory;
    }

    RegisteredModelsDisplayNameMap const &registeredModelsDisplayNameAssociation() const
    {
        return _registeredModelsDisplayName;
    }

    CategoriesSet const &categories() const { return _categories; }

    QString categoryDisplayName(QString const &category) const;

    void registerCategoryDisplayName(QString const &category, QString displayName)
    {
        _categoriesDisplayName[category] = std::move(displayName);
    }

private:
    RegisteredModelCreatorsMap _registeredItemCreators;

    RegisteredModelsCategoryMap _registeredModelsCategory;

    RegisteredModelsDisplayNameMap _registeredModelsDisplayName;

    CategoriesSet _categories;

    CategoriesDisplayNameMap _categoriesDisplayName;

private:
    // If the registered ModelType class has the static member method
    // `static QString Name();`, use it. Otherwise use the non-static
    // method: `virtual QString name() const;`
    template<typename T, typename = void>
    struct HasStaticMethodName : std::false_type
    {};

    template<typename T>
    struct HasStaticMethodName<
        T,
        typename std::enable_if<std::is_same<decltype(T::Name()), QString>::value>::type>
        : std::true_type
    {};

    template<typename ModelType>
    static QString computeName(RegistryItemCreator const &creator)
    {
        if constexpr (HasStaticMethodName<ModelType>::value) {
            return ModelType::Name();
        } else {
            return creator()->name();
        }
    }

    // If the registered ModelType class has the static member method
    // `static QString DisplayName();`, use it. Otherwise use the non-static
    // method: `virtual QString displayName() const;`
    template<typename T, typename = void>
    struct HasStaticMethodDisplayName : std::false_type
    {};

    template<typename T>
    struct HasStaticMethodDisplayName<
        T,
        typename std::enable_if<std::is_same<decltype(T::DisplayName()), QString>::value>::type>
        : std::true_type
    {};

    template<typename ModelType>
    static QString computeDisplayName(RegistryItemCreator const &creator)
    {
        if constexpr (HasStaticMethodDisplayName<ModelType>::value) {
            return ModelType::DisplayName();
        } else {
            return creator()->displayName();
        }
    }

    template<typename T>
    struct UnwrapUniquePtr
    {
        // Assert always fires, but the compiler doesn't know this:
        static_assert(!std::is_same<T, T>::value,
                      "The ModelCreator must return a std::unique_ptr<T>, where T "
                      "inherits from NodeDelegateModel");
    };

    template<typename T>
    struct UnwrapUniquePtr<std::unique_ptr<T>>
    {
        static_assert(std::is_base_of<NodeDelegateModel, T>::value,
                      "The ModelCreator must return a std::unique_ptr<T>, where T "
                      "inherits from NodeDelegateModel");
        using type = T;
    };

    template<typename CreatorResult>
    using compute_model_type_t = typename UnwrapUniquePtr<CreatorResult>::type;
};

} // namespace QtNodes
