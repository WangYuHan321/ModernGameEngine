class ConfigureManager
{
private:

	void LoadConfigure();

public:
	static ConfigureManager& GetMgr()
	{
		static ConfigureManager mgr;
		return mgr;
	}
};