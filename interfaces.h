class BackHandler {
public:
  virtual void handleBack() = 0;
  virtual void handleMenu() = 0;
  virtual void handleBackWithQuery(std::string, std::string) = 0;
  virtual void handleQuery(std::string, std::string) = 0;
};