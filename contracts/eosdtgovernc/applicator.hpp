#include "votes.hpp"



class appliator : public votes {

protected:
    void apply_ctract_setting(const ds_string &json) {
        struct ctrsetting {
            ds_ulong setting_id;
            uint8_t global_lock;
            ds_long time_shift;

            ds_account liquidator_account;
            ds_account oraclize_account;
            ds_account sttoken_account;
            ds_account nutoken_account;

            double governance_fee;
            double stability_fee;
            double critical_ltv;
            double liquidation_penalty;
            double liquidator_discount;
            ds_asset liquidation_price;
            double nut_auct_ratio;
            double nut_discount;
            double reserve_ratio;
            ds_uint vote_period;
            ds_uint stake_period;

            ds_ulong primary_key() const { return setting_id; }
        };
        auto eosdtcntract_account = settings_get().eosdtcntract_account;
        eosio::multi_index<"ctrsettings"_n, ctrsetting> ctrsettings(eosdtcntract_account, eosdtcntract_account.value);
        auto itr = ctrsettings.find(0);
        ds_assert(itr != ctrsettings.end(), "Need to set up % on ctract by forum.", SETTINGS);
        auto apply_settings = *itr;
        auto parser = get_json_parser(json.c_str());
        auto parse_status = parser.parse();
        auto settings_changed = false;
        auto producers_changed = false;
        auto apply_producers = std::vector<ds_account>{};
        for ( ; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (parser.is_key_equals("eosdtcntract.governance_fee")) {
                apply_settings.governance_fee = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.stability_fee")) {
                apply_settings.stability_fee = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.critical_ltv")) {
                apply_settings.critical_ltv = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.liquidation_penalty")) {
                apply_settings.liquidation_penalty = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.liquidator_discount")) {
                apply_settings.liquidator_discount = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.nut_auct_ratio")) {
                apply_settings.nut_auct_ratio = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.nut_discount")) {
                apply_settings.nut_discount = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.reserve_ratio")) {
                apply_settings.reserve_ratio = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.vote_period")) {
                apply_settings.vote_period = parser.get_out_double();
                settings_changed = true;
            }
            else if (parser.is_key_equals("eosdtcntract.stake_period")) {
                apply_settings.stake_period = parser.get_out_double();
                settings_changed = true;
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END,"\r\nproposal_json is invalid, code %", parse_status);
        if(settings_changed) {
            eosio::action(
                    eosio::permission_level{eosdtcntract_account, "active"_n},
                    eosdtcntract_account,
                    "settingset"_n,
                    std::make_tuple(std::optional<ds_account>(apply_settings.liquidator_account),
                                    std::optional<ds_account>(apply_settings.oraclize_account),
                                    std::optional<ds_account>(apply_settings.sttoken_account),
                                    std::optional<ds_account>(apply_settings.nutoken_account),
                                    std::optional<double>(apply_settings.governance_fee),
                                    std::optional<double>(apply_settings.stability_fee),
                                    std::optional<double>(apply_settings.critical_ltv),
                                    std::optional<double>(apply_settings.liquidation_penalty),
                                    std::optional<double>(apply_settings.liquidator_discount),
                                    std::optional<double>(apply_settings.nut_auct_ratio),
                                    std::optional<double>(apply_settings.nut_discount),
                                    std::optional<double>(apply_settings.reserve_ratio),
                                    std::optional<ds_uint>(apply_settings.vote_period),
                                    std::optional<ds_uint>(apply_settings.stake_period))
            ).send();
        }
    }

    void apply_ctract_producers(const std::map<std::string,ds_asset>& vote_producers) {
        ds_assert(vote_producers.size() > 0,"\r\nproposal wasn't accepted");
        std::vector<ds_account> apply_producers;
        for (auto prod = vote_producers.begin(); prod != vote_producers.end(); prod++) {
            apply_producers.push_back(ds_account{prod->first});
        }
        auto eosdtcntract_account = settings_get().eosdtcntract_account;
        eosio::action(
                eosio::permission_level{eosdtcntract_account, "active"_n},
                "eosdtcntract"_n,
                "setproducers"_n,
                std::make_tuple(apply_producers)).send();
    }

    void apply_liqdatr_setting(const ds_string &json) {
        if(json.find("eosdliqdatr")==ds_string::npos)
        {
            return;
        }
        struct liqdatr_settings {
            ds_ulong setting_id;
            ds_account eosdtcntract_account;
            uint8_t global_unlock;
            ds_asset auction_price;

            ds_ulong primary_key() const { return setting_id; }
        };
        auto liquidator_account = settings_get().liquidator_account;
        eosio::multi_index<"liqsettings"_n, liqdatr_settings> settings(liquidator_account, liquidator_account.value);
        auto itr = settings.find(0);
        ds_assert(itr != settings.end(), "Need to set up % on liqdatr by forum.", SETTINGS);
        auto apply_struct = *itr;
        auto parser = get_json_parser(json.c_str());
        auto parse_status = parser.parse();
        auto change_exists = false;
        for ( ; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (parser.is_key_equals("eosdliqdatr.eosdtcntract_account")) {
                apply_struct.eosdtcntract_account = parser.get_out_name();
                change_exists = true;
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END,"\r\nproposal_json is invalid, code %", parse_status);
        if(change_exists) {
            eosio::action(
                    eosio::permission_level{liquidator_account, "active"_n},
                    liquidator_account,
                    "settingset"_n,
                    std::make_tuple(apply_struct.eosdtcntract_account)
            ).send();
        }
    }

    void apply_oracle_setting(const ds_string &json) {
        if(json.find("eosdtorclize")==ds_string::npos)
        {
            return;
        }
        ds_print("\r\napply_oracle_setting");
        struct oracle_settings
        {
            ds_ulong id;
            ds_time utility_listing_date;
            ds_int rate_timeout;
            ds_int query_timeout;
            ds_int master_interval;
            ds_int slave_interval;

            ds_ulong primary_key() const { return id; }
        };
        auto oraclize_account = settings_get().oraclize_account;
        eosio::multi_index<"orasettings"_n, oracle_settings> settings(oraclize_account, oraclize_account.value);
        auto itr = settings.find(0);
        ds_assert(itr != settings.end(), "Need to set up % on oracle by forum.", SETTINGS);
        auto apply_struct = *itr;
        auto parser = get_json_parser(json.c_str());
        auto parse_status = parser.parse();
        auto change_exists = false;
        for ( ; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (parser.is_key_equals("eosdtorclize.rate_timeout")) {
                ds_print("\r\nrate_timeout");
                apply_struct.rate_timeout = parser.get_out_int();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtorclize.query_timeout")) {
                apply_struct.query_timeout = parser.get_out_int();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtorclize.master_interval")) {
                apply_struct.master_interval = parser.get_out_int();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtorclize.slave_interval")) {
                apply_struct.slave_interval = parser.get_out_int();
                change_exists = true;
            }

        }
        ds_assert(parse_status == json_parser::STATUS_END,"\r\nproposal_json is invalid, code %", parse_status);
        if(change_exists) {
            eosio::action(
                    eosio::permission_level{oraclize_account, "active"_n},
                    oraclize_account,
                    "settingset"_n,
                    std::make_tuple(
                            apply_struct.rate_timeout,
                            apply_struct.query_timeout,
                            apply_struct.master_interval,
                            apply_struct.slave_interval
                                    )
            ).send();
        }

    }

    void apply_governc_setting(const ds_string &json) {
        if(json.find("eosdtgovernc")==ds_string::npos)
        {
            return;
        }
        auto apply_struct = settings_get();
        auto parser = get_json_parser(json.c_str());
        auto parse_status = parser.parse();
        auto change_exists = false;
        for ( ; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (parser.is_key_equals("eosdtgovernc.eosdtcntract_account")) {
                apply_struct.eosdtcntract_account = parser.get_out_name();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.liquidator_account")) {
                apply_struct.liquidator_account = parser.get_out_name();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.oraclize_account")) {
                apply_struct.oraclize_account = parser.get_out_name();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.nutoken_account")) {
                apply_struct.nutoken_account = parser.get_out_name();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.min_proposal_weight")) {
                apply_struct.min_proposal_weight = parser.get_out_asset();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.freeze_period")) {
                apply_struct.freeze_period = parser.get_out_int();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.min_participation")) {
                apply_struct.min_participation = parser.get_out_double();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.success_margin")) {
                apply_struct.success_margin = parser.get_out_double();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.top_holders_amount")) {
                apply_struct.top_holders_amount = parser.get_out_int();
                change_exists = true;
            }
            else if (parser.is_key_equals("eosdtgovernc.min_threshold")) {
                apply_struct.min_threshold = parser.get_out_int();
                change_exists = true;
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END,"\r\nproposal_json is invalid, code %", parse_status);
        if(change_exists) {
            eosio::action(
                    eosio::permission_level{_self, "active"_n},
                    _self,
                    "settingset"_n,
                    std::make_tuple(apply_struct.eosdtcntract_account,
                                    apply_struct.liquidator_account,
                                    apply_struct.oraclize_account,
                                    apply_struct.nutoken_account,
                                    apply_struct.min_proposal_weight,
                                    apply_struct.freeze_period,
                                    apply_struct.min_participation,
                                    apply_struct.success_margin,
                                    apply_struct.top_holders_amount,
                                    std::optional<double>(apply_struct.min_threshold)
                    )
            ).send();
        }
    }

public:
    appliator(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            votes(receiver, code, ds) {
    }

    void apply(const ds_account &proposal_name) {
        PRINT_STARTED("apply"_n)
        auto proposal = proposal_get(proposal_name);
        auto settings = settings_get();
        auto freeze_period = settings.freeze_period;
        auto time = time_get();
        ds_assert(proposal.expires_at <= time, "proposal % did not expire, need to wait %.",
                  proposal_name, proposal.expires_at);
        ds_assert(proposal.expires_at + freeze_period > time,
                  "too late for apply proposal % freeze_period ended at %[%].",
                  proposal_name, proposal.expires_at + freeze_period, time);

        ds_int count_all, count_yes, count_no;
        ds_asset quantity_all, quantity_yes, quantity_no;
        std::map<std::string,ds_asset> vote_producers;
        vote_count(proposal_name,
                   count_all, count_yes, count_no,
                   quantity_all, quantity_yes, quantity_no, vote_producers);

        ds_assert(count_all > 0, "proposal % does not have any vote", proposal_name);

        auto total_nut_supply = supply_get(UTILITY_SYMBOL);
        ds_assert(total_nut_supply.amount>0,"need to issue some nuts.");
        auto min_participation = to_ldouble(quantity_all)/to_double(total_nut_supply);
        ds_assert(min_participation >= settings.min_participation-0.0001, "min_participation (%) less than necessary % for proposal %.",
                  min_participation, settings.min_participation, proposal_name);


        auto success_margin = (long double)quantity_yes.amount / (long double)quantity_all.amount;
        ds_assert(success_margin >= settings.success_margin-0.0001, "success_margin (%) less than necessary % for proposal %.",
                  success_margin, settings.success_margin, proposal_name);


        if(proposal.proposal_json.find("eosdtcntract")!=ds_string::npos)
        {
            if (proposal.proposal_type == 1) {
                apply_ctract_setting(proposal.proposal_json);
            }
            else if (proposal.proposal_type == 2) {
                apply_ctract_producers(vote_producers);
            }
        }

        apply_liqdatr_setting(proposal.proposal_json);

        apply_oracle_setting(proposal.proposal_json);

        apply_governc_setting(proposal.proposal_json);

        PRINT_FINISHED("apply"_n)
    }

};
