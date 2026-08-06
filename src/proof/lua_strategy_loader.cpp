#include "ravel/proof/lua_strategy_loader.hpp"

#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace ravel::proof {
namespace {

class LuaState {
public:
    LuaState() : L(luaL_newstate()) {
        if (!L) throw std::runtime_error("luaL_newstate failed");
        luaL_openlibs(L);
    }
    ~LuaState() { lua_close(L); }
    lua_State* get() const { return L; }
private:
    lua_State* L;
};

std::string field_string(lua_State* L, int index, const char* key) {
    index = lua_absindex(L, index);
    lua_getfield(L, index, key);
    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        throw std::runtime_error(std::string("expected string field '") + key + "'");
    }
    std::string value = lua_tostring(L, -1);
    lua_pop(L, 1);
    return value;
}

std::vector<std::string> field_string_array(lua_State* L, int index, const char* key) {
    index = lua_absindex(L, index);
    lua_getfield(L, index, key);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        throw std::runtime_error(std::string("expected table field '") + key + "'");
    }
    std::vector<std::string> out;
    const lua_Integer n = luaL_len(L, -1);
    for (lua_Integer i = 1; i <= n; ++i) {
        lua_geti(L, -1, i);
        if (!lua_isstring(L, -1)) {
            lua_pop(L, 2);
            throw std::runtime_error(std::string("expected string in '") + key + "'");
        }
        out.emplace_back(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return out;
}


bool field_bool(lua_State* L, int index, const char* key) {
    index = lua_absindex(L, index);
    lua_getfield(L, index, key);
    if (!lua_isboolean(L, -1)) {
        lua_pop(L, 1);
        throw std::runtime_error(std::string("expected boolean field '") + key + "'");
    }
    const bool value = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return value;
}

ReflectionConfig parse_reflection(lua_State* L, int schema_index) {
    schema_index = lua_absindex(L, schema_index);
    lua_getfield(L, schema_index, "reflection");
    if (lua_isnil(L, -1)) { lua_pop(L, 1); return {}; }
    if (!lua_istable(L, -1)) throw std::runtime_error("schema.reflection must be a table");
    ReflectionConfig config;
    config.enabled = field_bool(L, -1, "enabled");
    config.mode = field_string(L, -1, "mode");
    config.pivots = field_string_array(L, -1, "pivots");
    config.outputs = field_string_array(L, -1, "outputs");
    lua_pop(L, 1);
    return config;
}

void prepend_package_path(lua_State* L, const std::filesystem::path& root) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    const std::string old_path = lua_tostring(L, -1);
    lua_pop(L, 1);
    const std::string prefix = (root / "?.lua").string() + ";" +
                               (root / "?/init.lua").string() + ";";
    lua_pushstring(L, (prefix + old_path).c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
}

void require_module(lua_State* L, const std::string& module) {
    lua_getglobal(L, "require");
    lua_pushstring(L, module.c_str());
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        const std::string msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        throw std::runtime_error("cannot require " + module + ": " + msg);
    }
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        throw std::runtime_error(module + " did not return a table");
    }
}

std::vector<ProofObligation> parse_obligations(lua_State* L, int schema_index) {
    schema_index = lua_absindex(L, schema_index);
    lua_getfield(L, schema_index, "obligations");
    if (!lua_istable(L, -1)) throw std::runtime_error("schema.obligations must be a table");
    std::vector<ProofObligation> out;
    const lua_Integer n = luaL_len(L, -1);
    for (lua_Integer i = 1; i <= n; ++i) {
        lua_geti(L, -1, i);
        if (!lua_istable(L, -1)) throw std::runtime_error("obligation must be a table");
        out.push_back({field_string(L, -1, "id"), field_string(L, -1, "kind"),
                       field_string(L, -1, "statement"), field_string_array(L, -1, "requires")});
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return out;
}

PivotCourse parse_course(lua_State* L, int index) {
    return {
        field_string(L, index, "id"), field_string(L, index, "title"),
        field_string(L, index, "method"), field_string(L, index, "success"),
        field_string_array(L, index, "outputs"),
        field_string_array(L, index, "next_on_success"),
        field_string_array(L, index, "next_on_failure"),
        field_string_array(L, index, "failure_signatures"),
    };
}

std::unordered_map<std::string, ObligationStrategy> parse_strategies(lua_State* L, int tree_index) {
    tree_index = lua_absindex(L, tree_index);
    lua_getfield(L, tree_index, "obligations");
    if (!lua_istable(L, -1)) throw std::runtime_error("strategy.obligations must be a table");
    std::unordered_map<std::string, ObligationStrategy> out;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (!lua_isstring(L, -2) || !lua_istable(L, -1))
            throw std::runtime_error("strategy obligation entries must be string -> table");
        std::string id = lua_tostring(L, -2);
        ObligationStrategy strategy;
        strategy.objective = field_string(L, -1, "objective");
        lua_getfield(L, -1, "courses");
        if (!lua_istable(L, -1)) throw std::runtime_error(id + ".courses must be table");
        const lua_Integer n = luaL_len(L, -1);
        for (lua_Integer i = 1; i <= n; ++i) {
            lua_geti(L, -1, i);
            strategy.courses.push_back(parse_course(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
        out.emplace(std::move(id), std::move(strategy));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return out;
}

} // namespace

UniversalProofPlan load_universal_proof_plan(const std::filesystem::path& lua_root,
                                              const std::string& schema_module,
                                              const std::string& strategy_module) {
    LuaState state;
    lua_State* L = state.get();
    prepend_package_path(L, lua_root);

    require_module(L, schema_module);
    const int schema_index = lua_gettop(L);
    UniversalProofPlan plan;
    plan.schema_id = field_string(L, schema_index, "id");
    plan.obligations = parse_obligations(L, schema_index);
    plan.reflection = parse_reflection(L, schema_index);

    require_module(L, strategy_module);
    const int strategy_index = lua_gettop(L);
    plan.strategy_id = field_string(L, strategy_index, "id");
    plan.strategies = parse_strategies(L, strategy_index);
    return plan;
}

void validate_universal_proof_plan(const UniversalProofPlan& plan) {
    if (plan.obligations.empty()) throw std::runtime_error("proof schema has no obligations");
    if (plan.reflection.enabled) {
        if (plan.reflection.mode != "observe_and_prove")
            throw std::runtime_error("unsupported reflection mode: " + plan.reflection.mode);
        if (plan.reflection.pivots.empty())
            throw std::runtime_error("enabled reflection requires at least one pivot");
        if (plan.reflection.outputs.empty())
            throw std::runtime_error("enabled reflection requires at least one output");
    }
    std::unordered_set<std::string> obligation_ids;
    std::unordered_set<std::string> course_ids;
    for (const auto& obligation : plan.obligations) {
        if (!obligation_ids.insert(obligation.id).second)
            throw std::runtime_error("duplicate obligation: " + obligation.id);
    }
    for (const auto& obligation : plan.obligations) {
        for (const auto& dep : obligation.dependencies) {
            if (!obligation_ids.contains(dep))
                throw std::runtime_error(obligation.id + " requires unknown obligation " + dep);
        }
        const auto it = plan.strategies.find(obligation.id);
        if (it == plan.strategies.end())
            throw std::runtime_error("missing strategy for " + obligation.id);
        if (it->second.courses.size() < 2)
            throw std::runtime_error(obligation.id + " needs at least two pivot courses");
        for (const auto& course : it->second.courses) {
            if (!course_ids.insert(course.id).second)
                throw std::runtime_error("duplicate course: " + course.id);
        }
    }
    for (const auto& [obligation_id, strategy] : plan.strategies) {
        if (!obligation_ids.contains(obligation_id))
            throw std::runtime_error("strategy for unknown obligation " + obligation_id);
        for (const auto& course : strategy.courses) {
            for (const auto& pivot : course.next_on_failure) {
                if (!course_ids.contains(pivot))
                    throw std::runtime_error(course.id + " pivots to unknown course " + pivot);
            }
        }
    }

    std::unordered_map<std::string, int> marks;
    std::function<void(const std::string&)> visit = [&](const std::string& id) {
        if (marks[id] == 1) throw std::runtime_error("obligation dependency cycle at " + id);
        if (marks[id] == 2) return;
        marks[id] = 1;
        const auto it = std::find_if(plan.obligations.begin(), plan.obligations.end(),
                                     [&](const ProofObligation& x) { return x.id == id; });
        for (const auto& dep : it->dependencies) visit(dep);
        marks[id] = 2;
    };
    for (const auto& obligation : plan.obligations) visit(obligation.id);
}

} // namespace ravel::proof
