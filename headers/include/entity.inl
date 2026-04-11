template <typename T>
void entity::addScript() {
    T* s = new T();
    s->parent = this;
    scripts.push_back(s);
    contain[3] = true;
}