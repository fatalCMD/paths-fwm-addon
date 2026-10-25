// Path of the Companions: a perk tree for each follower role.
//
// Keeps every companion's perk points, ranks and carried abilities, and runs the abilities, the
// unique traits, the levels earned by fighting and the path bonuses, with a panel of its own to
// open the trees from. With Fight With Me installed, it gives each companion their role (so the
// path that counts) and steers the fight: what the abilities need of it is handed over through
// the interface. Without it, the same code steers for the abilities itself.

#include "FWM/Battle.h"
#include "FWM/Camera.h"
#include "FWM/Game.h"
#include "FWM/Effects.h"
#include "FWM/Forms.h"
#include "FWM/Freeze.h"
#include "FWM/Ripostes.h"
#include "FWM/Host.h"
#include "FWM/Log.h"
#include "FWM/LoadoutPolicy.h"
#include "FWM/Menu.h"
#include "FWM/OrdersLink.h"
#include "FWM/Overrides.h"
#include "FWM/PathBonus.h"
#include "FWM/Paths.h"
#include "FWM/PathsLink.h"
#include "FWM/Save.h"
#include "FWM/Settings.h"
#include "FWM/Text.h"
#include "FWM/TimeBend.h"
#include "FWM/Traits.h"
#include "FWM/WalkWithMe.h"
#include "FWM/WindstepDecoys.h"

#include "FightWithMePathsAPI.h"

namespace
{
	using namespace std::literals;
	namespace Paths = FWM::Paths;
	using FWM::Role;

	constexpr auto kName = "FightWithMePaths"sv;
	constexpr REL::Version kVersion{ FWMP_VERSION_MAJOR, FWMP_VERSION_MINOR, FWMP_VERSION_PATCH };

	template <class... Args>
	void Log(std::format_string<Args...> a_fmt, Args&&... a_args)
	{
		FWM::Log::Info(a_fmt, std::forward<Args>(a_args)...);
	}

	// --- Records ------------------------------------------------------------------------

	// Every companion's record, each with their own points. The rules for both live in Paths.
	std::mutex  storeLock;
	Paths::Book book;

	// --- Settings -----------------------------------------------------------------------

	std::filesystem::path PluginFolder()
	{
		std::error_code ec;
		return std::filesystem::current_path(ec) / L"Data" / L"SKSE" / L"Plugins";
	}

	// The shipped file, then what was changed in game, which no update replaces.
	std::wstring IniPath() { return (PluginFolder() / L"FightWithMePaths.ini").wstring(); }
	std::wstring UserIniPath() { return (PluginFolder() / L"FightWithMePaths_user.ini").wstring(); }

	int ReadInt(const wchar_t* a_key, int a_default)
	{
		const int shipped = static_cast<int>(::GetPrivateProfileIntW(L"Points", a_key, a_default, IniPath().c_str()));
		return static_cast<int>(::GetPrivateProfileIntW(L"Points", a_key, shipped, UserIniPath().c_str()));
	}

	void LoadSettings()
	{
		auto& points = book.points;
		points.above = std::clamp(ReadInt(L"iPointsAboveLevel", 10), 0, 100);
		points.extra = std::clamp(ReadInt(L"iBonusPoints", 0), 0, 100);
		points.respec = ReadInt(L"bAllowRespec", 1) != 0;
		points.unlimited = ReadInt(L"bAllowUnlimitedPoints", 1) != 0;
		Log("Settings: a point for every level past {}, {} bonus point(s), giving points back {}.", points.above, points.extra, points.respec ? "allowed" : "off");
	}

	FWM::Menu::PathsRules ReadRules()
	{
		const std::scoped_lock guard(storeLock);
		const auto&            points = book.points;
		return { points.above, points.extra, points.respec, points.unlimited };
	}

	// Into use at once, and into the user file: only what differs from the shipped one, a key at a
	// time, the way Fight With Me keeps its own.
	bool WriteRules(const FWM::Menu::PathsRules& a_rules)
	{
		{
			const std::scoped_lock guard(storeLock);
			auto&                  points = book.points;
			points.above = std::clamp(a_rules.above, 0, 100);
			points.extra = std::clamp(a_rules.extra, 0, 100);
			points.respec = a_rules.respec;
			points.unlimited = a_rules.unlimited;
		}
		const auto      user = UserIniPath();
		std::error_code ec;
		if (!std::filesystem::exists(user, ec)) {
			std::ofstream out(std::filesystem::path(user), std::ios::binary);
			out << "; Path of the Companions: the settings you changed in game, read after\r\n"
				   "; FightWithMePaths.ini. Updates never replace this file. Delete a line, or the whole\r\n"
				   "; file, to go back to what FightWithMePaths.ini says.\r\n\r\n";
			if (!out) {
				Log("Could not create FightWithMePaths_user.ini.");
				return false;
			}
		}
		const std::array<std::pair<const wchar_t*, int>, 4> keys{ {
			{ L"iPointsAboveLevel", a_rules.above },
			{ L"iBonusPoints", a_rules.extra },
			{ L"bAllowRespec", a_rules.respec ? 1 : 0 },
			{ L"bAllowUnlimitedPoints", a_rules.unlimited ? 1 : 0 },
		} };
		const std::array<int, 4> shipped{ static_cast<int>(::GetPrivateProfileIntW(L"Points", L"iPointsAboveLevel", 10, IniPath().c_str())),
			static_cast<int>(::GetPrivateProfileIntW(L"Points", L"iBonusPoints", 0, IniPath().c_str())),
			static_cast<int>(::GetPrivateProfileIntW(L"Points", L"bAllowRespec", 1, IniPath().c_str())) != 0 ? 1 : 0,
			static_cast<int>(::GetPrivateProfileIntW(L"Points", L"bAllowUnlimitedPoints", 1, IniPath().c_str())) != 0 ? 1 : 0 };
		bool ok = true;
		for (std::size_t i = 0; i < keys.size(); ++i) {
			const auto value = std::to_wstring(keys[i].second);
			const bool mine = keys[i].second != shipped[i];
			ok = ::WritePrivateProfileStringW(L"Points", keys[i].first, mine ? value.c_str() : nullptr, user.c_str()) && ok;
		}
		Log("Settings changed: a point for every level past {}, {} bonus point(s), giving points back {}, unlimited points {}{}.", a_rules.above, a_rules.extra,
			a_rules.respec ? "allowed" : "off", a_rules.unlimited ? "offered" : "not offered", ok ? "" : "; FightWithMePaths_user.ini could not be written");
		return ok;
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
			if(role==Role::kBruiser && !FWM::Game::ArsenalOf(RE::TESForm::LookupByID<RE::Actor>(a_actor)).twoHanded) return static_cast<int>(Paths::Block::kBadNode);
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

		std::uint32_t Steering(FightWithMePathsAPI::Steer* a_out, std::uint32_t a_capacity) override
		{
			if (!a_out) {
				return 0;
			}
			std::uint32_t written = 0;
			for (const auto& steer : FWM::Battle::Steering()) {
				if (written >= a_capacity) {
					break;
				}
				auto& out = a_out[written++];
				out = {};
				out.actor = steer.member;
				out.target = steer.target.native_handle();
				out.circle = steer.circle.native_handle();
				out.radius = steer.radius;
				out.bash = steer.tweaks.bash;
				out.power = steer.tweaks.power;
				out.flank = steer.tweaks.flank;
				out.circling = steer.tweaks.circle;
				out.fallback = steer.tweaks.fallback;
				out.strafe = steer.tweaks.strafe;
				out.avoidThreat = steer.tweaks.avoidThreat;
				out.offensive = steer.tweaks.offensive;
				out.defensive = steer.tweaks.defensive;
			}
			return written;
		}

		std::uint32_t Holds(FightWithMePathsAPI::Hold* a_out, std::uint32_t a_capacity) override
		{
			if (!a_out) {
				return 0;
			}
			std::uint32_t written = 0;
			for (const auto& [enemy, tank] : FWM::Battle::Holds()) {
				if (written >= a_capacity) {
					break;
				}
				a_out[written++] = { enemy.native_handle(), tank.native_handle() };
			}
			return written;
		}

		void Adopt(void* a_serialization, std::uint32_t a_type, std::uint32_t a_version, const std::uint8_t* a_data, std::uint32_t a_size) override
		{
			auto* intfc = static_cast<SKSE::SerializationInterface*>(a_serialization);
			if (!intfc || (!a_data && a_size)) {
				return;
			}
			FWM::Save::Adopt(intfc, a_type, a_version, std::vector<std::uint8_t>(a_data, a_data + a_size));
		}

		void DrawInPlan(FightWithMePathsAPI::Plan* a_plan) override
		{
			if (a_plan) {
				FWM::Menu::DrawInPlan(*a_plan);
			}
		}
	} api;

	// --- Co-save ------------------------------------------------------------------------
	// The paths, and everything the abilities keep between saves (their lifts, their cooldowns,
	// the levels earned by fighting, each companion's switches), in the records Fight With Me once
	// wrote into its own co-save. On its own, also each companion's path and what they may hold.

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
		FWM::Save::Write(a_intfc);
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
				if (version < 3 && !loaded.empty()) {
					Log("Progression update: reset {} companion paths and refunded all earned points; trait ranks now require levels 25/50.", loaded.size());
				}
			} else if (type == kBeforeType) {
				kept = Paths::Decode(bytes, version);
			} else {
				FWM::Save::Read(a_intfc, type, version, bytes);
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
		{
			const std::scoped_lock guard(storeLock);
			book.Clear();
		}
		FWM::Save::Revert();
	}

	// --- On its own ---------------------------------------------------------------------

	struct PlayerUpdate
	{
		static void Thunk(RE::PlayerCharacter* a_player, float a_delta)
		{
			original(a_player, a_delta);
			FWM::Battle::Update(a_delta);
		}

		static void Install()
		{
			REL::Relocation<std::uintptr_t> vtable{ RE::PlayerCharacter::VTABLE[0] };
			original = vtable.write_vfunc(0xAD, Thunk);
			Log("[Hook] Player update installed.");
		}

		static inline REL::Relocation<decltype(Thunk)> original;
	};

	// The words in the player's language, from the folder Fight With Me reads them from too.
	void LoadText()
	{
		std::string language = FWM::Settings::Get().language;
		if (language.empty()) {
			if (auto* setting = RE::GetINISetting("sLanguage:General"); setting && setting->GetType() == RE::Setting::Type::kString && setting->GetString()) {
				language = setting->GetString();
			}
		}
		language = FWM::Text::Upper(language.empty() ? "ENGLISH"sv : std::string_view(language));
		std::error_code ec;
		const auto      folder = std::filesystem::current_path(ec) / L"Data" / L"SKSE" / L"Plugins" / L"FightWithMe" / L"Translations";
		const auto      loaded = FWM::Text::Load(folder, language);
		if (loaded.files.empty()) {
			Log("[Text] {}: no translation files, so English.", language);
			return;
		}
		Log("[Text] {}: {} lines from {} file(s).", language, loaded.entries, loaded.files.size());
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_message)
	{
		if (!a_message) {
			return;
		}
		switch (a_message->type) {
		case SKSE::MessagingInterface::kPostLoad:
			if (::GetModuleHandleW(L"FightWithMe.dll")) {
				Log("Fight With Me is here: it gives each companion their role and steers the fight; this runs the abilities, traits and levels.");
				FWM::OrdersLink::Connect();
			} else {
				FWM::Host::SetPathsAlone();
				Log("Fight With Me is not installed: Path of the Companions also steers the fight for its abilities.");
				FWM::WalkWithMe::Connect();
			}
			FWM::Settings::Load();
			FWM::PathsLink::Connect();
			FWM::Camera::RegisterSmoothCam();
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			LoadText();
			FWM::Forms::Resolve();
			FWM::Traits::LoadNotes();
			FWM::Effects::Install();
			// Beside Fight With Me, its hook steers and its pause menu is already registered.
			if (FWM::Host::PathsAlone()) {
				FWM::TimeBend::Register();
				FWM::Overrides::Install();
			}
			FWM::PathBonus::Install();
			PlayerUpdate::Install();
			// This DLL owns the perk menu and its camera state, including when paired.
			FWM::Camera::Install();
			FWM::Camera::RequestSmoothCam();
			FWM::Freeze::Install();
            FWM::Ripostes::Install();
			FWM::Battle::InstallEvents();
			FWM::Menu::RegisterPaths({ ReadRules, WriteRules });
			break;
		case SKSE::MessagingInterface::kNewGame:
		case SKSE::MessagingInterface::kPreLoadGame:
			FWM::Battle::Reset();
			break;
		default:
			break;
		}
	}
}

extern "C" FIGHTWITHMEPATHS_API bool FightWithMePaths_IsWindstepDecoy(std::uint32_t actor)
{
    return FWM::WindstepDecoys::Is(actor);
}

extern "C" FIGHTWITHMEPATHS_API std::uint32_t FightWithMePaths_GetLoadoutMask(std::uint32_t role)
{
    return role < static_cast<std::uint32_t>(FWM::Role::kCount) ?
        FWM::LoadoutPolicy::RoleWeapons(static_cast<FWM::Role>(role)) : 0;
}

extern "C" FIGHTWITHMEPATHS_API FightWithMePathsAPI::IPaths* FightWithMePaths_GetInterface(std::uint32_t a_version)
{
	return a_version == FightWithMePathsAPI::INTERFACE_VERSION ? &api : nullptr;
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	FWM::Log::Setup(L"FightWithMePaths");
	Log("Path of the Companions {} loading; trees {:08X}.", kVersion.string("."sv), Paths::Signature());
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
