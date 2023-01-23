#pragma once
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>
#include <algorithm>
#include <initializer_list>
#include <iostream>

/* Ревьюеру:
 * В некоторых случаях я обращаюсь к полям и методам класса через this->,
 * в других - прямо. Это связано с тем, что я для себя не определил,
 * какой способ лучше для читабельности кода.
 */

template <typename Type>
class SingleLinkedList {
    // Узел списка
    struct Node {
        Node() = default;
        Node(const Type& val, Node* next)
            : value(val)
            , next_node(next) {
        }
        Type value;
        Node* next_node = nullptr;
    };
    
    // Шаблон класса «Базовый Итератор».
    // Определяет поведение итератора на элементы односвязного списка
    // ValueType — совпадает с Type (для Iterator) либо с const Type (для ConstIterator)
    template <typename ValueType>
    class BasicIterator {
    public:
        // Объявленные ниже типы сообщают стандартной библиотеке о свойствах этого итератора

        // Категория итератора — forward iterator
        // (итератор, который поддерживает операции инкремента и многократное разыменование)
        using iterator_category = std::forward_iterator_tag;
        // Тип элементов, по которым перемещается итератор
        using value_type = Type;
        // Тип, используемый для хранения смещения между итераторами
        using difference_type = std::ptrdiff_t;
        // Тип указателя на итерируемое значение
        using pointer = ValueType*;
        // Тип ссылки на итерируемое значение
        using reference = ValueType&;

        BasicIterator() = default;

        // Конвертирующий конструктор/конструктор копирования
        // При ValueType, совпадающем с Type, играет роль копирующего конструктора
        // При ValueType, совпадающем с const Type, играет роль конвертирующего конструктора
        BasicIterator(const BasicIterator<Type>& other) noexcept {
            this->node_ = other.node_;
        }

        // Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
        // пользовательского конструктора копирования, явно объявим оператор = и
        // попросим компилятор сгенерировать его за нас
        BasicIterator& operator=(const BasicIterator& rhs) = default;

        // Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка либо на end()
        [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            return this->node_ == rhs.node_;
        }

        // Оператор проверки итераторов на неравенство
        // Противоположен !=
        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
            return !(*this == rhs);
        }

        // Оператор сравнения итераторов (в роли второго аргумента итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка либо на end()
        [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
            return this->node_ == rhs.node_;
        }

        // Оператор проверки итераторов на неравенство
        // Противоположен !=
        [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
            return !(*this == rhs);
        }

        // Оператор прединкремента. После его вызова итератор указывает на следующий элемент списка
        // Возвращает ссылку на самого себя
        // Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
        BasicIterator& operator++() noexcept {
            node_ = node_->next_node;
            return *this;
        }

        // Оператор постинкремента. После его вызова итератор указывает на следующий элемент списка
        // Возвращает прежнее значение итератора
        // Инкремент итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        BasicIterator operator++(int) noexcept {
            auto old_value(*this);
            ++(*this);
            return old_value;
        }

        // Операция разыменования. Возвращает ссылку на текущий элемент
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] reference operator*() const noexcept {
            return node_->value;
        }

        // Операция доступа к члену класса. Возвращает указатель на текущий элемент списка
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] pointer operator->() const noexcept {
            return &(node_->value);
        }

    private:
        // Класс списка объявляется дружественным, чтобы из методов списка
        // был доступ к приватной области итератора
        friend class SingleLinkedList;

        // Конвертирующий конструктор итератора из указателя на узел списка
        explicit BasicIterator(Node* node) {
            node_ = node;
        }
        
        Node* node_ = nullptr;
    };

public:
    SingleLinkedList() {}
    
    /* Конструкторы-----------------------------------------------------------------*/
    SingleLinkedList(std::initializer_list<Type> values) {
        if (size_ != 0 || head_.next_node != nullptr) {
            return;
        }
    
        SingleLinkedList<Type> tmp;
        Node* current_node = &(tmp.head_);
        for (const Type& value : values) {
            current_node->next_node = new Node(value, nullptr);
            current_node = current_node->next_node;
            ++(tmp.size_);
        }
        this->swap(tmp);
    }
    
    SingleLinkedList(const SingleLinkedList& other) {
        if (size_ != 0 && head_.next_node != nullptr) {
            return;
        }
    
        SingleLinkedList<Type> tmp;
        Node* current_node = &(tmp.head_);
        for (const Type& value : other) {
            current_node->next_node = new Node(value, nullptr);
            current_node = current_node->next_node;
            ++(tmp.size_);
        }
        this->swap(tmp);
    }
    /*----------------------------------------------------------------- Конструкторы*/
    
    SingleLinkedList& operator=(const SingleLinkedList<Type>& other) {
        if (this == &other) {
            return *this;
        }
        
        SingleLinkedList<Type> tmp(other);
        this->swap(tmp);
        return *this;
    }
    
    /* Итераторы--------------------------------------------------------------------*/
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;

    // Итератор, допускающий изменение элементов списка
    using Iterator = BasicIterator<Type>;
    // Константный итератор, предоставляющий доступ для чтения к элементам списка
    using ConstIterator = BasicIterator<const Type>;

    // Возвращает итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен end()
    [[nodiscard]] Iterator begin() noexcept {
        if (head_.next_node) {
            return Iterator{head_.next_node};
        }
        return this->end();
    }

    // Возвращает итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя — попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] Iterator end() noexcept {
        return Iterator{nullptr};
    }

    // Возвращает константный итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен end()
    // Результат вызова эквивалентен вызову метода cbegin()
    [[nodiscard]] ConstIterator begin() const noexcept {
        return this->cbegin();
    }

    // Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя — попытка разыменования приведёт к неопределённому поведению
    // Результат вызова эквивалентен вызову метода cend()
    [[nodiscard]] ConstIterator end() const noexcept {
        return this->cend();
    }

    // Возвращает константный итератор, ссылающийся на первый элемент
    // Если список пустой, возвращённый итератор будет равен cend()
    [[nodiscard]] ConstIterator cbegin() const noexcept {
        if (head_.next_node) {
            return ConstIterator{head_.next_node};
        }
        return this->cend();
    }

    // Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
    // Разыменовывать этот итератор нельзя — попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator cend() const noexcept {
        return ConstIterator{nullptr};
    }
    
    // Возвращает итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] Iterator before_begin() noexcept {
        return Iterator{&(this->head_)};
    }

    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        return ConstIterator{const_cast<Node*>(&(this->head_))};
    }

    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator before_begin() const noexcept {
        return this->cbefore_begin();
    }
    /*-------------------------------------------------------------------- Итераторы*/
    
    // Вставляет элемент value в начало списка за время O(1)
    void PushFront(const Type& value) {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }
    
    /*
     * Вставляет элемент value после элемента, на который указывает pos.
     * Возвращает итератор на вставленный элемент
     * Если при создании элемента будет выброшено исключение, список останется в прежнем состоянии
     */
    Iterator InsertAfter(ConstIterator pos, const Type& value) {
        Node* new_node = new Node(value, pos.node_->next_node);
        pos.node_->next_node = new_node;
        ++(this->size_);
        return Iterator{new_node};
    }
    
    void PopFront() noexcept {
        if (0u == this->size_ || nullptr == this->head_.next_node) {
            return;
        }
        
        Node* node_after = this->head_.next_node->next_node;
        delete this->head_.next_node;
        this->head_.next_node = node_after;
        --(this->size_);
    }
    
    /*
     * Удаляет элемент, следующий за pos.
     * Возвращает итератор на элемент, следующий за удалённым
     */
    Iterator EraseAfter(ConstIterator pos) noexcept {
        if (nullptr == pos.node_->next_node) {
            return Iterator{pos.node_};
        }
        
        Node* node_after = pos.node_->next_node->next_node;
        delete pos.node_->next_node;
        pos.node_->next_node = node_after;
        --(this->size_);
        return Iterator{node_after};
    }
    
    void swap(SingleLinkedList<Type>& list_to_swap) noexcept {
        Node* first_elem_list = list_to_swap.head_.next_node;
        size_t size_list = list_to_swap.size_;
        
        list_to_swap.head_.next_node = this->head_.next_node;
        list_to_swap.size_ = this->size_;
        this->head_.next_node = first_elem_list;
        this->size_ = size_list;
    }

    // Возвращает количество элементов в списке за время O(1)
    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }

    // Сообщает, пустой ли список за время O(1)
    [[nodiscard]] bool IsEmpty() const noexcept {
        // Заглушка. Реализуйте метод самостоятельно
        return size_ ? false : true;
    }
    
    ~SingleLinkedList() {
        Clear();
    }
    
    // Очищает список за время O(N)
    void Clear() noexcept {
        while (head_.next_node) {
            Node* node_to_remove = head_.next_node;
            head_.next_node = node_to_remove->next_node;
            delete node_to_remove;
        }
        size_ = 0u;
    }

private:
    // Фиктивный узел, используется для вставки "перед первым элементом"
    Node head_;
    size_t size_ = 0u;
};

template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

/* Операторы сравнения-----------------------------------------------------------*/

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) noexcept {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) noexcept {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) noexcept {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) noexcept {
    return lhs < rhs || lhs == rhs;
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) noexcept {
    return !(lhs < rhs);
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) noexcept {
    return lhs > rhs || lhs == rhs;
}
    
/*----------------------------------------------------------- Операторы сравнения*/
