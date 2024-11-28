#pragma once

#include "document.h"
#include "search_server.h"

#include <deque>
#include <string>
#include <vector>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentPredicate document_predicate);   
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    
    int GetNoResultRequests() const;
    
private:
    struct QueryResult {
        int time;
        int matches_number;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& search_server_;
    int current_time_ = 0;
    int empty_responses_ = 0;    
}; 

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query,
                                                   DocumentPredicate document_predicate) {
    const auto matched_documents = search_server_.FindTopDocuments(raw_query, document_predicate);
    ++current_time_;
    int matches_number = static_cast<int>(matched_documents.size());
    if (!requests_.empty() && current_time_ - requests_.front().time >= min_in_day_) {
        if (requests_.front().matches_number == 0) {
            --empty_responses_;
        }
        requests_.pop_front();
    }
    requests_.push_back({current_time_, matches_number});
    if (!matches_number) {
        ++empty_responses_;
    }
    return matched_documents;
}