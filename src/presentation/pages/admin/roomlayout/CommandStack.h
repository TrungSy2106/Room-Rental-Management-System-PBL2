#ifndef COMMANDSTACK_H
#define COMMANDSTACK_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct ICommand {
    virtual ~ICommand() = default;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual std::string description() const { return {}; }
};

class CommandStack {
public:
    CommandStack() = default;
    CommandStack(const CommandStack &) = delete;
    CommandStack &operator=(const CommandStack &) = delete;

    void push(std::unique_ptr<ICommand> cmd);
    void undo();
    void redo();

    bool canUndo() const noexcept { return !undoStack_.empty(); }
    bool canRedo() const noexcept { return !redoStack_.empty(); }

    void clear();
    void setClean();
    bool isClean() const noexcept { return cleanIndex_ == static_cast<int>(undoStack_.size()); }

    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

private:
    void notify();

    std::vector<std::unique_ptr<ICommand>> undoStack_;
    std::vector<std::unique_ptr<ICommand>> redoStack_;
    int cleanIndex_ = 0;
    std::function<void()> onChange_;
};

#endif
