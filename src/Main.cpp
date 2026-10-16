// Paths - Follower Ability Skilltree: a perk tree for each follower role in Fight With Me.
//
// Keeps every companion's perk points, ranks and carried abilities, and saves them in the
// co-save. Fight With Me draws the trees in its battle plan and carries the abilities out;
// this plugin is the record of what each companion has learned, and the rules for learning
// more. Without Fight With Me it does nothing.

#include "FWM/Paths.h"

#include "FightWithMePathsAPI.h"

namespace
{
	using namespace std::literals;
	namespace Paths = FWM::Paths;
	using FWM::Role;

	constexpr auto kName = "FightWithMePaths"sv;
	constexpr REL::Version kVersion{ FWMP_VERSION_MAJOR, FWMP_VERSION_MINOR, FWMP_VERSION_PATCH };

	// --- Log ----------------------------------------------------------------------------

	std::mutex    logLock;
	std::ofstream logFile;

	void OpenLog()
	{
		try {
			const auto directory = SKSE::log::log_directory();
			if (!directory) {
				return;
			}
			std::error_code ec;
			std::filesystem::create_directories(*directory, ec);
			const auto current = *directory / L"FightWithMePaths.log";
			if (std::filesystem::exists(current, ec)) {
				std::filesystem::rename(current, *directory / L"FightWithMePaths.previous.log", ec);
			}
			logFile.open(current, std::ios::out | std::ios::trunc);
		} catch (...) {
		}
	}

	template <class... Args>
	void Log(std::format_string<Args...> a_fmt, Args&&... a_args)
	{
		const std::scoped_lock guard(logLock);
		if (!logFile.is_open()) {
			return;
		}
		SYSTEMTIME now{};
		::GetLocalTime(&now);
		logFile << std::format("[{:02}:{:02}:{:02}.{:03}] ", now.wHour, now.wMinute, now.wSecond, now.wMilliseconds)
				<< std::format(a_fmt, std::forward<Args>(a_args)...) << '\n';
		logFile.flush();
	}

	// --- Settings -----------------------------------------------------------------------

	// --- Records ------------------------------------------------------------------------

	// Every companion's record, each with their own points. The rules for both live in Paths.
	std::mutex  storeLock;
	Paths::Book book;

	void LoadSettings()
	{
		std::error_code ec;
		const auto      path = (std::filesystem::current_path(ec) / L"Data" / L"SKSE" / L"Plugins" / L"FightWithMePaths.ini").wstring();
		auto&           points = book.points;
		points.above = std::clamp(static_cast<int>(::GetPrivateProfileIntW(L"Points", L"iPointsAboveLevel", 10, path.c_str())), 0, 100);
		points.extra = std::clamp(static_cast<int>(::GetPrivateProfileIntW(L"Points", L"iBonusPoints", 0, path.c_str())), 0, 100);
		points.respec = ::GetPrivateProfileIntW(L"Points", L"bAllowRespec", 1, path.c_str()) != 0;
		points.unlimited = ::GetPrivateProfileIntW(L"Points", L"bAllowUnlimitedPoints", 1, path.c_str()) != 0;
		Log("Settings: a point for every level past {}, {} bonus point(s), giving points back {}.", points.above, points.extra, points.respec ? "allowed" : "off");
	}

	std::string NameOf(RE::FormID a_id)
	{
		auto*       actor = RE::TESForm::LookupByID<RE::Actor>(a_id);
		const char* name = actor ? actor->GetDisplayFullName() : nullptr;
		return name && *name ? std::string(name) : std::format("{:08X}", a_id);
	}

	// The record, made on first sight. A companion met above the level points start at has
	// every point that level earned waiting for them; their own level, nobody else's.
	const Paths::Record* Find(RE::FormID a_id, bool a_create)
	{
		auto*      actor = RE::TESForm::LookupByID<RE::Actor>(a_id);
		const auto known = book.Find(a_id) != nullptr;
		if (!known && (!a_create || !actor)) {
			return nullptr;
		}
		const int   had = known ? book.EarnedAt(book.Find(a_id)->peak) : 0;
		const int   level = actor ? std::max(static_cast<int>(actor->GetLevel()), 1) : 0;
		const auto* record = book.Meet(a_id, level);
		if (!record) {
			return nullptr;
		}
		if (!known) {
			Log("{}: first seen at level {}; {} perk point(s) to spend.", NameOf(a_id), record->peak, book.Earned(a_id));
		} else if (const int after = book.EarnedAt(record->peak); after > had) {
			Log("{} reached level {}: {} more perk point(s).", NameOf(a_id), record->peak, after - had);
		}
		return record;
	}

	void Fill(RE::FormID a_id, const Paths::Record& a_record, FightWithMePathsAPI::Record& a_out)
	{
		a_out = {};
		for (std::size_t r = 0; r < Paths::kRoles; ++r) {
			for (std::size_t i = 0; i < Paths::kNodes; ++i) {
				a_out.ranks[r][i] = a_record.ranks[r][i];
			}
			for (std::size_t s = 0; s < Paths::kSlots; ++s) {
				a_out.slots[r][s] = static_cast<std::uint8_t>(a_record.slots[r][s]);
			}
			a_out.bonus[r] = a_record.bonus[r];
		}
		a_out.peak = a_record.peak;
		a_out.earned = book.Earned(a_id);
		a_out.flags = book.Unlimited(a_id) ? FightWithMePathsAPI::kUnlimitedFlag : 0;
		a_out.spent = Paths::Spent(a_record);
	}

	std::string_view NodeName(Role a_role, std::size_t a_node)
	{
		return a_node < Paths::kNodes ? Paths::PathOf(a_role)[a_node].name : "?"sv;
	}

	// --- Interface ----------------------------------------------------------------------

	class Interface final : public FightWithMePathsAPI::IPaths
	{
	public:
		std::uint32_t GetVersion() const noexcept override { return FightWithMePathsAPI::INTERFACE_VERSION; }

		std::uint32_t GetTreeSignature() const noexcept override { return Paths::Signature(); }

		bool Read(std::uint32_t a_actor, FightWithMePathsAPI::Record* a_out) override
		{
			if (!a_out) {
				return false;
			}
			const std::scoped_lock guard(storeLock);
			auto*                  record = Find(a_actor, true);
			if (!record) {
				return false;
			}
			Fill(a_actor, *record, *a_out);
			return true;
		}

		FightWithMePathsAPI::Result Learn(std::uint32_t a_actor, std::uint32_t a_role, std::uint32_t a_node) override
		{
			if (a_role >= Paths::kRoles) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const std::scoped_lock guard(storeLock);
			const auto*            record = Find(a_actor, true);
			if (!record) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const auto role = static_cast<Role>(a_role);
			const auto block = book.Learn(a_actor, role, a_node);
			if (block == Paths::Block::kNone) {
				Log("{} learns {} (rank {}, {} of {} point(s) spent).", NameOf(a_actor), NodeName(role, a_node), record->ranks[a_role][a_node],
					Paths::Spent(*record), book.Earned(a_actor));
			}
			return static_cast<int>(block);
		}

		FightWithMePathsAPI::Result Forget(std::uint32_t a_actor, std::uint32_t a_role, std::uint32_t a_node) override
		{
			if (a_role >= Paths::kRoles) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const std::scoped_lock guard(storeLock);
			const auto*            record = Find(a_actor, false);
			const auto             role = static_cast<Role>(a_role);
			const auto             block = book.Forget(a_actor, role, a_node);
			if (block == Paths::Block::kNone && record) {
				Log("{} gives back a rank of {} (now {}).", NameOf(a_actor), NodeName(role, a_node), record->ranks[a_role][a_node]);
			}
			return static_cast<int>(block);
		}

		FightWithMePathsAPI::Result Reset(std::uint32_t a_actor, std::uint32_t a_role) override
		{
			if (a_role >= Paths::kRoles) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const std::scoped_lock guard(storeLock);
			const auto*            record = Find(a_actor, false);
			const auto             role = static_cast<Role>(a_role);
			const int              back = record ? Paths::Spent(*record, role) : 0;
			const int              bonus = record ? Paths::Invested(*record, role) : 0;
			const auto             block = book.Reset(a_actor, role);
			if (block == Paths::Block::kNone) {
				Log("{} starts the {} over: {} point(s) back{}.", NameOf(a_actor), Paths::Info(role).title, back,
					bonus ? std::format(", {} of them from its bonus", bonus) : ""s);
			}
			return static_cast<int>(block);
		}

		FightWithMePathsAPI::Result Equip(std::uint32_t a_actor, std::uint32_t a_role, std::uint32_t a_slot, std::uint32_t a_ability) override
		{
			if (a_role >= Paths::kRoles || a_ability >= Paths::kAbilityCount) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const std::scoped_lock guard(storeLock);
			const auto*            record = Find(a_actor, true);
			if (!record) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const auto block = book.Equip(a_actor, static_cast<Role>(a_role), a_slot, static_cast<Paths::Ability>(a_ability));
			if (block == Paths::Block::kNone) {
				const auto& slots = record->slots[a_role];
				Log("{} carries {} and {}.", NameOf(a_actor), Paths::Info(slots[0]).name, Paths::Info(slots[1]).name);
			}
			return static_cast<int>(block);
		}

		std::int32_t PointsAboveLevel() const override { return book.points.above; }

		bool RespecAllowed() const override { return book.points.respec; }

		FightWithMePathsAPI::Result SetUnlimited(std::uint32_t a_actor, bool a_on) override
		{
			const std::scoped_lock guard(storeLock);
			const auto*            record = Find(a_actor, true);
			if (!record) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const bool on = book.Unlimited(a_actor);
			const int  spent = Paths::Spent(*record);
			const auto block = book.SetUnlimited(a_actor, a_on);
			if (block != Paths::Block::kNone || a_on == on) {
				return static_cast<int>(block);
			}
			if (a_on) {
				Log("{} has unlimited points now ({} of {} spent before).", NameOf(a_actor), spent, book.EarnedAt(record->peak));
			} else {
				Log("{} is back to earned points: {} spent with unlimited points, {} now.", NameOf(a_actor), spent, Paths::Spent(*record));
			}
			return static_cast<int>(block);
		}

		bool UnlimitedAllowed() const override { return book.points.unlimited; }

		FightWithMePathsAPI::Result Invest(std::uint32_t a_actor, std::uint32_t a_role, std::int32_t a_room) override
		{
			if (a_role >= Paths::kRoles) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const std::scoped_lock guard(storeLock);
			const auto*            record = Find(a_actor, true);
			if (!record) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const auto role = static_cast<Role>(a_role);
			const auto block = book.Invest(a_actor, role, a_room);
			if (block == Paths::Block::kNone) {
				Log("{} puts a point into the {}'s bonus ({} there now, {} of {} point(s) spent).", NameOf(a_actor), Paths::Info(role).title,
					Paths::Invested(*record, role), Paths::Spent(*record), book.Earned(a_actor));
			}
			return static_cast<int>(block);
		}

		FightWithMePathsAPI::Result Divest(std::uint32_t a_actor, std::uint32_t a_role) override
		{
			if (a_role >= Paths::kRoles) {
				return static_cast<int>(Paths::Block::kBadNode);
			}
			const std::scoped_lock guard(storeLock);
			const auto*            record = Find(a_actor, false);
			const auto             role = static_cast<Role>(a_role);
			const auto             block = book.Divest(a_actor, role);
			if (block == Paths::Block::kNone && record) {
				Log("{} takes a point back out of the {}'s bonus ({} there now).", NameOf(a_actor), Paths::Info(role).title, Paths::Invested(*record, role));
			}
			return static_cast<int>(block);
		}
	} api;

	// --- Co-save ------------------------------------------------------------------------

	constexpr std::uint32_t kUniqueID = 'FWMP';
	constexpr std::uint32_t kRecordType = 'PATH';
	constexpr std::uint32_t kBeforeType = 'UNLM';  // companions with unlimited points: their record before

	bool WriteEntries(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type, const std::vector<Paths::Entry>& a_entries)
	{
		const auto bytes = Paths::Encode(a_entries);
		return a_intfc->OpenRecord(a_type, Paths::kRecordVersion) && a_intfc->WriteRecordData(bytes.data(), static_cast<std::uint32_t>(bytes.size()));
	}

	void OnSave(SKSE::SerializationInterface* a_intfc)
	{
		std::vector<Paths::Entry> entries, kept;
		{
			const std::scoped_lock guard(storeLock);
			entries = book.Records();
			kept = book.KeptAside();
		}
		if (!WriteEntries(a_intfc, kRecordType, entries)) {
			Log("Could not write the paths of {} companion(s).", entries.size());
			return;
		}
		if (!kept.empty() && !WriteEntries(a_intfc, kBeforeType, kept)) {
			Log("Could not write the records kept aside for {} companion(s) with unlimited points.", kept.size());
		}
		Log("Paths of {} companion(s) saved{}.", entries.size(), kept.empty() ? ""s : std::format(", {} with unlimited points", kept.size()));
	}

	// Both records are read before anything is set right: a companion with unlimited points keeps
	// every rank they bought with them.
	void OnLoad(SKSE::SerializationInterface* a_intfc)
	{
		std::vector<Paths::Entry> loaded, kept;
		std::uint32_t             type = 0, version = 0, length = 0;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			std::vector<std::uint8_t> bytes(length);
			if (length && a_intfc->ReadRecordData(bytes.data(), length) != length) {
				Log("A record is short.");
				continue;
			}
			if (type == kRecordType) {
				loaded = Paths::Decode(bytes, version);
			} else if (type == kBeforeType) {
				kept = Paths::Decode(bytes, version);
			}
		}
		auto resolve = [&](RE::FormID a_id) {
			RE::FormID now = 0;
			return a_intfc->ResolveFormID(a_id, now) ? now : 0;
		};
		int dropped = 0;
		for (auto& [id, record] : loaded) {
			id = resolve(id);
			dropped += id ? 0 : 1;  // the follower's plugin is gone
		}
		for (auto& [id, record] : kept) {
			id = resolve(id);
		}
		const std::scoped_lock guard(storeLock);
		const int              repaired = book.Load(std::move(loaded), std::move(kept));
		const auto             records = book.Records();
		const auto             unlimited = book.KeptAside().size();
		Log("Paths of {} companion(s) loaded{}{}{}.", records.size(), dropped ? std::format(", {} from removed plugins dropped", dropped) : ""s,
			repaired ? std::format(", {} did not fit the trees or the points as they are now and were set right", repaired) : ""s,
			unlimited ? std::format(", {} with unlimited points", unlimited) : ""s);
	}

	void OnRevert(SKSE::SerializationInterface*)
	{
		const std::scoped_lock guard(storeLock);
		book.Clear();
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_message)
	{
		if (a_message && a_message->type == SKSE::MessagingInterface::kDataLoaded) {
			const bool fightWithMe = ::GetModuleHandleW(L"FightWithMe.dll") != nullptr;
			Log("Data loaded; Fight With Me {}.", fightWithMe ? "is here" : "is NOT loaded, so nothing will use these paths");
		}
	}
}

extern "C" FIGHTWITHMEPATHS_API FightWithMePathsAPI::IPaths* FightWithMePaths_GetInterface(std::uint32_t a_version)
{
	return a_version == FightWithMePathsAPI::INTERFACE_VERSION ? &api : nullptr;
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	OpenLog();
	Log("Paths - Follower Ability Skilltree {} loading; trees {:08X}.", kVersion.string("."sv), Paths::Signature());
	SKSE::Init(a_skse, false);
	if (REL::Module::IsVR()) {
		Log("Skyrim VR is not supported.");
		return false;
	}
	LoadSettings();
	auto* serialization = SKSE::GetSerializationInterface();
	if (!serialization) {
		Log("Serialization interface unavailable; paths will not persist.");
		return false;
	}
	serialization->SetUniqueID(kUniqueID);
	serialization->SetSaveCallback(OnSave);
	serialization->SetLoadCallback(OnLoad);
	serialization->SetRevertCallback(OnRevert);
	if (auto* messaging = SKSE::GetMessagingInterface()) {
		messaging->RegisterListener(OnMessage);
	}
	return true;
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData data;
	data.PluginName(kName);
	data.PluginVersion(kVersion);
	data.AuthorName("Fight With Me");
	data.UsesAddressLibrary();
	data.UsesNoStructs();
	return data;
}();

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* a_info)
{
	a_info->name = SKSEPlugin_Version.pluginName;
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->version = SKSEPlugin_Version.pluginVersion;
	return true;
}
